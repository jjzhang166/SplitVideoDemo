#include "SplitVideo.h"

#include <QDebug>

using namespace std;

SplitVideo::SplitVideo(QObject *parent) : QObject(parent)
{

}

void SplitVideo::setInOutName(string infile, string outfile)
{
    inputFileName = infile;
    outputFileName = outfile;
    suffixName = ".mp4";
}

SplitVideo::~SplitVideo()
{

}


bool SplitVideo::executeSplit(unsigned int beginTime, unsigned int endTime)
{
    AVPacket readPkt;
    int ret;
    av_register_all();

    if ((ret = avformat_open_input(&ifmtCtx, inputFileName.c_str(), 0, 0)) < 0) {
        return false;
    }

    if ((ret = avformat_find_stream_info(ifmtCtx, 0)) < 0) {
        return false;
    }

    for (unsigned int i = 0; i < ifmtCtx->nb_streams; i++) {
        AVStream *in_stream = ifmtCtx->streams[i];
        if (in_stream->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            video_index = i;
        }
    }
    //int den = ifmtCtx->streams[0]->avg_frame_rate.den;
    //int num = ifmtCtx->streams[0]->avg_frame_rate.num;
    int den = ifmtCtx->streams[video_index]->r_frame_rate.den;
    int num = ifmtCtx->streams[video_index]->r_frame_rate.num;
    float fps = (float)num / den;
    unsigned int beginFPS = fps*beginTime;
    unsigned int endFPS = fps*endTime;

    string save_name;
    save_name = outputFileName.substr(0, outputFileName.find_last_of("."));
    string temp_name = save_name + "" +suffixName;
    avformat_alloc_output_context2(&ofmtCtx, NULL, NULL, temp_name.c_str());
    if (!ofmtCtx){
        return false;
    }

    if (!writeVideoHeader(ifmtCtx, ofmtCtx, temp_name)){
        return false;
    }
    vector<uint64_t> vecEndFramePos;
    vector<uint64_t> vecBeginFramePos;
    uint64_t frame_index = 0;
    uint64_t beginFrame_index =0;
    uint64_t endFrame_index = 0;
    unsigned int beginFrameCount = 0;
    unsigned int endFrameCount = 0;
    //读取分割点附近的关键帧位置
    while (1){
        ++frame_index;
        ret = av_read_frame(ifmtCtx, &readPkt);
        if (ret < 0){
            break;
        }
        //过滤，只处理视频流
        if (readPkt.stream_index == video_index){
            ++beginFrameCount;
            ++endFrameCount;
            if (readPkt.flags==AV_PKT_FLAG_KEY){
                beginFrame_index = frame_index;
                endFrame_index = frame_index;
            }
            if (endFrameCount>endFPS){
                vecEndFramePos.push_back(endFrame_index);
                endFrameCount = 0;
            }
            if (beginFrameCount>beginFPS){
                vecBeginFramePos.push_back(beginFrame_index);
                beginFrameCount = 0;
            }
        }
        av_packet_unref(&readPkt);
    }

    qDebug() << beginFrame_index << "beginFrame_index";
    qDebug() << endFrame_index << "endFrame_index";

    avformat_close_input(&ifmtCtx);
    ifmtCtx = NULL;

    //为了重新获取avformatcontext
    if ((ret = avformat_open_input(&ifmtCtx, inputFileName.c_str(), 0, 0)) < 0) {
        return -1;
    }

    if ((ret = avformat_find_stream_info(ifmtCtx, 0)) < 0) {
        return -1;
    }

    if (vecEndFramePos.empty()){
        vecEndFramePos.push_back(frame_index);
    }
    if (vecBeginFramePos.empty()){
        vecBeginFramePos.push_back(frame_index);
    }


    vector<uint64_t>::iterator keyFrameIterEnd = vecEndFramePos.begin();
    vector<uint64_t>::iterator keyFrameIterBegin = vecBeginFramePos.begin();
    endFrame_index = *keyFrameIterEnd;
    beginFrame_index = *keyFrameIterBegin;
    ++keyFrameIterEnd;
    ++keyFrameIterBegin;
    frame_index = 0;

    int count = 0;
    while (1){
        ++frame_index;
        ret = av_read_frame(ifmtCtx, &readPkt);
        if (ret < 0){
            break;
        }

        av_packet_rescale_ts(&readPkt, ifmtCtx->streams[readPkt.stream_index]->time_base, ofmtCtx->streams[readPkt.stream_index]->time_base);

//        qDebug() << readPkt.pts << readPkt.dts << "+++";
        if (readPkt.pts < readPkt.dts){
            readPkt.pts = readPkt.dts + 1;
        }
//        qDebug() << beginFrame_index << "beginFrame_indexbeginFrame_index";
//        qDebug() << endFrame_index << "endFrame_indexendFrame_indexendFrame_index";
        if(frame_index >= beginFrame_index){
            if (readPkt.flags&AV_PKT_FLAG_KEY&&frame_index == endFrame_index){
            }
            else{
    //            qDebug() << count++  << "******************";
                qDebug() << readPkt.size << "readPkt.size "
                         << readPkt.duration << " readPkt.duration "
                         << readPkt.data << "readPkt.data "
                         << readPkt.dts << "readPkt.dts "
                         << readPkt.pts << "readPkt.pts "
                         ;
                if(readPkt.size > 0){
                }
                ret = av_interleaved_write_frame(ofmtCtx, &readPkt);
//                ret = av_write_frame(ofmtCtx, &readPkt);
            }

            if (frame_index >= endFrame_index){
                av_packet_unref(&readPkt);
                av_write_trailer(ofmtCtx);
                avformat_close_input(&ifmtCtx);
                avio_close(ofmtCtx->pb);
                avformat_free_context(ofmtCtx);
                avformat_free_context(ifmtCtx);
                vecResultName.push_back(temp_name);
                return true;
            }
        }
    }
    return true;
}

vector<string> SplitVideo::getResultName()
{
    return vecResultName;
}

bool SplitVideo::writeVideoHeader(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx, string out_filename)
{
    AVOutputFormat *ofmt = NULL;
    int ret;

    ofmt = ofmt_ctx->oformat;
    for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
        //根据输入流创建输出流（Create output AVStream according to input AVStream）
        AVStream *in_stream = ifmt_ctx->streams[i];
        if (in_stream->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            video_index = i;
        }
        AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
        if (!out_stream) {
            ret = AVERROR_UNKNOWN;
            return false;
        }
        //复制AVCodecContext的设置（Copy the settings of AVCodecContext）
        ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
        if (ret < 0) {
            return false;
        }
        out_stream->codec->codec_tag = 0;
        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

    }
    if (!(ofmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, out_filename.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            return false;
        }
    }
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0){
        return false;
    }
    return true;
}

void SplitVideo::av_packet_rescale_ts(AVPacket *pkt, AVRational src_tb, AVRational dst_tb)
{
    if (pkt->pts != AV_NOPTS_VALUE)
    pkt->pts = av_rescale_q(pkt->pts, src_tb, dst_tb);
    if (pkt->dts != AV_NOPTS_VALUE)
    pkt->dts = av_rescale_q(pkt->dts, src_tb, dst_tb);
    if (pkt->duration > 0)
    pkt->duration = av_rescale_q(pkt->duration, src_tb, dst_tb);
    if (pkt->convergence_duration > 0)
    pkt->convergence_duration = av_rescale_q(pkt->convergence_duration, src_tb, dst_tb);
}

#ifndef SPLITVIDEO_H
#define SPLITVIDEO_H

#ifndef INT64_C
#define INT64_C
#define UINT64_C
#endif

//引入ffmpeg头文件
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include <libavutil/frame.h>
}
#include <QObject>

using namespace std;

class SplitVideo : public QObject
{
    Q_OBJECT
public:
    explicit SplitVideo(QObject *parent = 0);
    void setInOutName(string infile,string outfile);
    ~SplitVideo();

    bool executeSplit(unsigned int beginTime, unsigned int endTime);
    vector<string> getResultName();

    void av_packet_rescale_ts(AVPacket *pkt, AVRational src_tb, AVRational dst_tb);
signals:

public slots:

private:
    uint64_t splitFrameSize;
    vector<string> vecResultName;
    string suffixName;
    string inputFileName;
    string outputFileName;
    int video_index,audio_index;
    AVFormatContext *ifmtCtx=NULL, *ofmtCtx=NULL;
    bool writeVideoHeader(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx, string out_filename);
    void flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index);
};

#endif // SPLITVIDEO_H

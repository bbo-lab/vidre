#include <vidre/config.hh>
#include <vidre/ffmpeg.hh>

using namespace vidre;

#include <QCoreApplication>
#include <QCommandLineParser>

#include <iostream>

#define tr(...) QCoreApplication::translate(__VA_ARGS__)

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("vidre-encode");
    QCoreApplication::setApplicationVersion(VIDRE_VERSION_S);

    // === command-line options
    QCommandLineParser parser;
    parser.setApplicationDescription("Utility for encoding raw videos.");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("input", tr("main", "Raw video input file."));
    parser.addPositionalArgument("output", tr("main", "Output file or directory."), "[output]");

    parser.addOptions({
        {"width", tr("main", "Frame width (pixels, default: 752)."), "width", "752"},
        {"height", tr("main", "Frame height (pixels, default: 480)."), "height", "480"},
        {"fps", tr("main", "Frames per second (default: 50)."), "fps", "50"},
        {"bpp", tr("main", "Bits per pixel (default: 10)."), "bpp", "10"},
    });

    parser.addOptions({
        {"codec", tr("main", "Output codec (default: h264)."), "codec", "h264"},
        {"quality", tr("main", "Output quality (default: 90)."), "quality", "90"},
    });

    // Process the actual command line arguments given by the user
    parser.process(app);

    // === run
    ffmpeg::dictionary dict = {{"keya", "valuea"}, {"keyb", "valueb"}};
    try {
        // FIXME
        throw ffmpeg::exception() << ffmpeg::errc_info(static_cast<ffmpeg::errc>(AVERROR_BUG));

        ffmpeg::format::context ctxt;
        ctxt.write_header({{"key", "value"}});

    } catch (vidre::exception const &e) {
        std::cerr << boost::diagnostic_information(e) << std::endl;
        return -1;
    }

    return 0;
}
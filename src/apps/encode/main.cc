#include <vidre/config.hh>

#include <QCoreApplication>
#include <QCommandLineParser>

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
    // FIXME
    return 0;
}
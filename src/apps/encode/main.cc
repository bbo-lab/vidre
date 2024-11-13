#include <vidre/config.hh>
#include <vidre/exception.hh>
#include <vidre/ffmpeg.hh>

#include "input/midraw.hh"

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
        {"mono", tr("main", "Monochrome (default: true)."), "bpp", "1"},
    });

    parser.addOptions({
        {"codec", tr("main", "Output codec (default: h264)."), "codec", "h264"},
        {"quality", tr("main", "Output quality (default: 90)."), "quality", "90"},
    });

    parser.addOptions({{"check", tr("main", "No output, only check file.")}});

    // Process the actual command line arguments given by the user
    parser.process(app);

    // === run
    const QStringList args = parser.positionalArguments();
    if (args.size() < 1) {
        qCritical("No input file name given.\nExiting...");
        return 1;
    }

    std::filesystem::path infile = args[0].toStdString();

    input::midraw in(infile);
    in.setformat(parser.value("width").toUInt(), parser.value("height").toUInt(),  //
                 parser.value("bpp").toUInt(), parser.value("mono").toInt());

    if (not in.check(parser.isSet("check"))) {
        qCritical("Input file check failed.\nExiting...");
        return 1;
    }

    if (parser.isSet("check"))
        return 0;

    auto outfile = infile;
    if (args.size() > 1)
        outfile = args[1].toStdString();
    else {
        outfile.replace_extension(".mp4");
        qInfo("saving to %s", outfile.c_str());
    }

    try {
        if (not in.open())
            throw vidre::exception() << throw_message("opening input file failed");

        while(not in.eof()) {
            auto &src = in.get_frame();
        }

    } catch (vidre::exception const &e) {
        std::cerr << boost::diagnostic_information(e) << std::endl;
        return 1;
    }

    // size_t fps = parser.value("fps").toULongLong();

    // ffmpeg::dictionary dict = {{"keya", "valuea"}, {"keyb", "valueb"}};
    // try {
    //     // FIXME
    //     throw ffmpeg::exception() << ffmpeg::errc_info(static_cast<ffmpeg::errc>(AVERROR_BUG));

    //     ffmpeg::format::context ctxt;
    //     ctxt.write_header({{"key", "value"}});

    // } catch (vidre::exception const &e) {
    //     std::cerr << boost::diagnostic_information(e) << std::endl;
    //     return -1;
    // }

    return 0;
}
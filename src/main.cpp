#include <QCoreApplication>

#include <qmlinstrumenttask.h>

#include <boost/program_options.hpp>
#include <ostream>

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
    // Declare the supported options.
    po::options_description desc(
                "qml-tools\n\n"
                "Instruments qml files for collecting coverage data\n\n"
                "Allowed options");
    desc.add_options()
            ("help", "shows this help message")
            ("input,i", po::value<std::string>(), "input file or folder")
            ("output,o", po::value<std::string>()->default_value(""), "output file or folder");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help") || !vm.count("input")) {
        std::cout << desc << "\n";
        return 1;
    }

    po::notify(vm);

    QmlInstrumentTask instrumenter;
    instrumenter.instrument(
            QString::fromStdString(vm["input"].as<std::string>()),
            QString::fromStdString(vm["output"].as<std::string>()));

    instrumenter.saveInitialCoverageData("coverage_data.json");
}

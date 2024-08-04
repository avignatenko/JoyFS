#include "ReadSettings.h"

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ini_parser.hpp>

using boost::property_tree::ptree;
using std::filesystem::file_time_type;
using std::filesystem::path;

ptree readSettings(const path& file)
{
    ptree settings;
    boost::property_tree::read_ini(file.string(), settings);
    return settings;
}

LogSettings readLogSettings(const ptree& settings)
{
    LogSettings logSettings;

    auto logLevelStr = boost::algorithm::to_lower_copy(settings.get<std::string>("App.LogLevel"));

    std::map<std::string, spdlog::level::level_enum> settingsStrToEnum = {
        {"trace", spdlog::level::trace}, {"debug", spdlog::level::debug}, {"info", spdlog::level::info},
        {"warn", spdlog::level::warn},   {"err", spdlog::level::err},     {"critical", spdlog::level::critical},
        {"off", spdlog::level::off}};

    if (auto levelFound = settingsStrToEnum.find(logLevelStr); levelFound != settingsStrToEnum.end())
        logSettings.logLevel = levelFound->second;

    return logSettings;
}

bffcl::UDPClient::Settings readCLSettings(const ptree& settings)
{
    bffcl::UDPClient::Settings clSettings;
    clSettings.toAddress = settings.get<std::string>("Network.CLIPAddress");
    clSettings.toPort = settings.get<int>("Network.CLPort"),
    clSettings.fromAddress = settings.get<std::string>("Network.ThisIPAddress"),
    clSettings.fromPort = settings.get<int>("Network.ThisPort");
    clSettings.sendFreq = settings.get<double>("Network.SendFreqHz");

    return clSettings;
}

Sim::Settings readSimSettings(const ptree& settings)
{
    Sim::Settings simSettings;
    simSettings.invertFSElevator = settings.get<bool>("Sim.InvertElevator");
    simSettings.invertFSAileron = settings.get<bool>("Sim.InvertAileron");
    simSettings.invertCLElevatorTrim = settings.get<bool>("Sim.InvertCLElevatorTrim");
    simSettings.propWashOffset = std::stoul(settings.get<std::string>("Sim.PropWashOffset"), nullptr, 16);
    simSettings.clElevatorTrimOffset = std::stoul(settings.get<std::string>("Sim.CLElevatorTrimOffset"), nullptr, 16);
    simSettings.apRollEngagedOffset = std::stoul(settings.get<std::string>("Sim.APRollEngagedOffset"), nullptr, 16);
    simSettings.apPitchEngagedOffset = std::stoul(settings.get<std::string>("Sim.APPitchEngagedOffset"), nullptr, 16);
    simSettings.apPitchLimitsOffset = std::stoul(settings.get<std::string>("Sim.APPitchLimitsOffset"), nullptr, 16);
    simSettings.clForceEnabledOffset = std::stoul(settings.get<std::string>("Sim.CLForceEnabledOffset"), nullptr, 16);
    simSettings.clEngageOffset = std::stoul(settings.get<std::string>("Sim.CLEngageOffset"), nullptr, 16);
    return simSettings;
}

A2AStec30AP::Settings readAPSettings(const ptree& settings)
{
    A2AStec30AP::Settings apSettings;

    apSettings.pitchmode = settings.get<int>("AP.PitchMode");

    apSettings.elevatorServoDuMax = settings.get<int>("AP.ElevatorServoDuMax");

    apSettings.elevatorPID.p = settings.get<double>("AP.ElevatorP");
    apSettings.elevatorPID.i = settings.get<double>("AP.ElevatorI");
    apSettings.elevatorPID.d = settings.get<double>("AP.ElevatorD");
    apSettings.elevatorDuMax = settings.get<double>("AP.ElevatorDuMax");

    apSettings.pitchPID.p = settings.get<double>("AP.PitchP");
    apSettings.pitchPID.i = settings.get<double>("AP.PitchI");
    apSettings.pitchPID.d = settings.get<double>("AP.PitchD");
    apSettings.pitchMax = degToRad(settings.get<double>("AP.PitchMaxDeg"));
    apSettings.pitchDuMax = settings.get<double>("AP.PitchDuMax");

    apSettings.fpmPID.p = settings.get<double>("AP.FpmP");
    apSettings.fpmPID.i = settings.get<double>("AP.FpmI");
    apSettings.fpmPID.d = settings.get<double>("AP.FpmD");
    apSettings.fpmMax = settings.get<double>("AP.FpmMax");
    apSettings.fpmDuMax = settings.get<double>("AP.FpmDuMax");

    apSettings.pitchWarningCLForce = settings.get<double>("AP.PitchWarningCLForce");
    apSettings.pitchMaxCLForce = settings.get<double>("AP.PitchMaxCLForce");
    apSettings.pitchStartDegradeCLForce = settings.get<double>("AP.PitchStartDegradeCLForce");

    apSettings.doStepResponse = settings.get<bool>("AP.DoStepResponse");
    apSettings.stepResponseInputFile = settings.get<std::string>("AP.StepResponseInputFile");
    return apSettings;
}

Model::Settings readModelSettings(const ptree& settings)
{
    Model::Settings modelSettings;
    modelSettings.hardware.aileronFrictionCoeff = settings.get<int>("Model.AileronFrictionCoeff");
    modelSettings.hardware.aileronDumpingCoeff = settings.get<int>("Model.AileronDumpingCoeff");
    modelSettings.hardware.aileronPositionFollowingP = settings.get<int>("Model.AileronPositionFollowingP");
    modelSettings.hardware.aileronPositionFollowingI = settings.get<int>("Model.AileronPositionFollowingI");
    modelSettings.hardware.aileronPositionFollowingD = settings.get<int>("Model.AileronPositionFollowingD");

    modelSettings.hardware.elevatorFrictionCoeff = settings.get<int>("Model.ElevatorFrictionCoeff");
    modelSettings.hardware.elevatorDumpingCoeff = settings.get<int>("Model.ElevatorDumpingCoeff");
    modelSettings.hardware.elevatorPositionFollowingP = settings.get<int>("Model.ElevatorPositionFollowingP");
    modelSettings.hardware.elevatorPositionFollowingI = settings.get<int>("Model.ElevatorPositionFollowingI");
    modelSettings.hardware.elevatorPositionFollowingD = settings.get<int>("Model.ElevatorPositionFollowingD");

    modelSettings.clExponent = settings.get<double>("Model.CLExponent");

    modelSettings.hTailPosLon = settings.get<double>("Model.HTailPosLon");
    modelSettings.wingRootChord = settings.get<double>("Model.WingRootChord");

    modelSettings.elevatorArea = settings.get<double>("Model.ElevatorArea");
    modelSettings.elevatorNeutralPos = settings.get<double>("Model.ElevatorNeutralPos");
    modelSettings.elevatorTrimGain = settings.get<double>("Model.ElevatorTrimGain");
    modelSettings.elevatorTrimNeutralPos = settings.get<double>("Model.ElevatorTrimNeutralPos");
    modelSettings.propWashElevatorCoeff = settings.get<double>("Model.PropWashElevatorCoeff");
    modelSettings.calculatePropWash = settings.get<bool>("Model.CalculatePropWash");

    modelSettings.elevatorAlphaGain = settings.get<double>("Model.ElevatorAlphaGain");
    modelSettings.elevatorAlphaScaleSpeedKn = settings.get<double>("Model.ElevatorAlphaScaleSpeedKn");

    modelSettings.elevatorPRGain = settings.get<double>("Model.ElevatorPRGain");
    modelSettings.maxElevatorLift = settings.get<double>("Model.MaxElevatorLift");
    modelSettings.maxElevatorAngleRadians = degToRad(settings.get<double>("Model.MaxElevatorAngleDegrees"));

    modelSettings.engineVibAirGain = settings.get<double>("Model.EngineVibAirGain");

    modelSettings.elevatorEngineFlowGain = settings.get<double>("Model.ElevatorEngineFlowGain");
    modelSettings.elevatorEngineFreqGain = settings.get<double>("Model.ElevatorEngineFreqGain");
    modelSettings.elevatorEngineFreqMin = settings.get<double>("Model.ElevatorEngineFreqMin");

    modelSettings.elevatorVibStallGain = settings.get<double>("Model.ElevatorVibStallGain");
    modelSettings.elevatorVibStalFreq = settings.get<double>("Model.ElevatorVibStalFreq");

    modelSettings.elevatorVibRunwayGain = settings.get<double>("Model.ElevatorVibRunwayGain");
    modelSettings.elevatorVibRunwayFreq = settings.get<double>("Model.ElevatorVibRunwayFreq");

    modelSettings.aileronArea = settings.get<double>("Model.AileronArea");
    modelSettings.aileronTrimGain = settings.get<double>("Model.AileronTrimGain");
    modelSettings.propWashAileronCoeff = settings.get<double>("Model.PropWashAileronCoeff");
    modelSettings.maxAileronLift = settings.get<double>("Model.MaxAileronLift");
    modelSettings.maxAileronAngleRadians = degToRad(settings.get<double>("Model.MaxAileronAngleDegrees"));

    modelSettings.aileronEngineFlowGain = settings.get<double>("Model.AileronEngineFlowGain");
    modelSettings.aileronEngineFreqGain = settings.get<double>("Model.AileronEngineFreqGain");
    modelSettings.aileronEngineFreqMin = settings.get<double>("Model.AileronEngineFreqMin");

    modelSettings.aileronVibStallGain = settings.get<double>("Model.AileronVibStallGain");
    modelSettings.aileronVibStalFreq = settings.get<double>("Model.AileronVibStalFreq");

    modelSettings.aileronVibRunwayGain = settings.get<double>("Model.AileronVibRunwayGain");
    modelSettings.aileronVibRunwayFreq = settings.get<double>("Model.AileronVibRunwayFreq");

    modelSettings.forceTrimIntoSim = settings.get<bool>("Model.ForceTrimIntoSim");
    modelSettings.forcedSimTrim = settings.get<double>("Model.ForcedSimTrim");

    return modelSettings;
}
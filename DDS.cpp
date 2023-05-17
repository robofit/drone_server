#include <iostream>
#include <exception>

#include <boost/asio.hpp>
#include <boost/exception/all.hpp>
#include <boost/program_options.hpp>

#include <DDS/config.hpp>
#include <DDS/core/logger.hpp>
#include <DDS/core/settings.hpp>
#include <DDS/core/media/manager.hpp>

#ifdef COMPILE_WEBSOCKET
#include <DDS/websocket/server.hpp>
std::shared_ptr<WebsocketServer> websocket_server_p;
#endif

#ifdef COMPILE_RECORDER
#include <DDS/recorder/recorder.hpp>

extern "C"
{
#include <libavformat/avformat.h>
}

#endif

#ifdef COMPILE_FLIGHT_DATABASE
#include <DDS/flight_database/controller.hpp>
#endif

#ifdef COMPILE_VEHICLE_DETECTION
#include <DDS/vehicle_detection/vehicle_detection.hpp>
#endif

#ifdef COMPILE_RTMP
#include <DDS/core/tcp_server.hpp>
#include <DDS/rtmp/rtmp_session.hpp>

class my_rtmp_session : public rtmp_session
{
public:
    my_rtmp_session(tcp::socket socket)
		: rtmp_session(std::move(socket)) {}
private:
    void on_stream_create(ClientID_t cid)
    {
        auto& sett = settings::get();
#ifdef COMPILE_RECORDER
        if(sett.dbool["record_force"])
            media_manager::get().pipe(cid)->add_writer(std::make_shared<media_recorder>(cid_to_hex(cid) + ".mp4"));
#endif

#ifdef COMPILE_VEHICLE_DETECTION
        if(sett.dbool["vehicle_detection_force"])
            media_manager::get().pipe(cid)->add_writer(std::make_shared<VehicleDetector>(sett.dstring["vehicle_detection_filepath"]));
#endif
    }
};

std::shared_ptr<tcp_server<my_rtmp_session>> rtmp_server_p;

#endif

int run()
{
    auto& sett = settings::get();

    boost::asio::io_context io_context;
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);

    signals.async_wait([&io_context](const boost::system::error_code& error, int signum)
    {
        if(signum == SIGINT)
        {
            LOG(INFO) << "<DDS> " << "SIGINT received. Stopping server.";
        }
        else if(signum == SIGTERM)
        {
            LOG(INFO) << "<DDS> " << "SIGTERM received. Stopping server.";
        }
        io_context.stop();
    });


#ifdef COMPILE_WEBSOCKET
    try
    {
        websocket_server_p = std::make_shared<WebsocketServer>(&io_context);
        websocket_server_p->run(sett.dint["websocket_port"]);
    }
    catch (websocketpp::exception const& e)
    {
        LOG(ERROR) << "<DDS> " << e.what();
        return -1;
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "<DDS> " << e.what();
        return -1;
    }
#endif

#ifdef COMPILE_RTMP
    try
    {
        rtmp_server_p = std::make_shared<tcp_server<my_rtmp_session>>(io_context);
        rtmp_server_p->run(sett.dint["rtmp_port"]);
    }
    catch (boost::exception &e)
    {
        LOG(ERROR) << "<DDS> " << boost::diagnostic_information(e);
        return -1;
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "<DDS> " << e.what();
        return -1;
    }
#endif

    try
    {
        io_context.run();
    }
    catch (boost::exception &e)
    {
        LOG(ERROR) << "<DDS> " << boost::diagnostic_information(e);
        return -1;
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "<DDS> " << e.what();
        return -1;
    }

    media_manager::get().clear();

    return 0;
}

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
    auto& sett = settings::get();

    int log_level;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("log,l", po::value<int>(&log_level)->default_value(2), "log level: 3-ERROR,2-WARNING,1-INFO,0-DEBUG")

#ifdef COMPILE_WEBSOCKET
        ("websocket_port,wp", po::value<int>(&sett.dint["websocket_port"])->default_value(5555), "Websocket Server port")
#endif

#ifdef COMPILE_RTMP
        ("rtmp_port,rp", po::value<int>(&sett.dint["rtmp_port"])->default_value(1935), "RTMP Server port")
#endif

#ifdef COMPILE_RECORDER
        ("rec_width,rcw", po::value<int>(&sett.dint["record_width"])->default_value(1920), "record width")
        ("rec_height,rch", po::value<int>(&sett.dint["record_height"])->default_value(1080), "record height")
        ("rec_fps,rcf", po::value<int>(&sett.dint["record_fps"])->default_value(20), "record fps")
        ("rec_bitrate,rcbr", po::value<int>(&sett.dint["record_bitrate"])->default_value(3500), "record bitrate in kb/s")
        ("rec_f,r", po::value<bool>(&sett.dbool["record_force"])->default_value(false), "force stream recording")
#endif

#ifdef COMPILE_FLIGHT_DATABASE
        ("fdb_h", po::value<std::string>(&sett.dstring["mariadb_hostname"])->default_value("localhost"), "mariadb host name")
        ("fdb_u", po::value<std::string>(&sett.dstring["mariadb_username"])->default_value("root"), "mariadb user name")
        ("fdb_p", po::value<std::string>(&sett.dstring["mariadb_password"])->default_value("root"), "mariadb password")
        ("fdb_s", po::value<std::string>(&sett.dstring["mariadb_database"])->default_value("DDS"), "mariadb scheme/database")
#endif

#ifdef COMPILE_VEHICLE_DETECTION
        ("vdet_fp", po::value<std::string>(&sett.dstring["vehicle_detection_filepath"])->default_value("cars.xml"), "vehicle detection cascade file path")
        ("vdet_f,d", po::value<bool>(&sett.dbool["vehicle_detection_force"])->default_value(false), "force vehicle detection")
#endif
    ;

    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    }
    catch (po::error_with_option_name &e)
    {
        LOG(ERROR) << "<DDS> " << e.what();
        return -1;
    }
    

#ifdef COMPILE_RECORDER
    sett.dint["record_av_codec_id"] = AVCodecID::AV_CODEC_ID_H264;
    sett.dint["record_av_pix_fmt_id"] = AVPixelFormat::AV_PIX_FMT_YUV420P;
    sett.dint["record_gop_size"] = 12;
#endif

    sett.set_loglevel(log_level);

    if(vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 0;
    }

    return run();
}

#include <DDS/rtmp/rtmp_session.hpp>
#include <DDS/rtmp/rtmp_stream.hpp>

#include <DDS/core/logger.hpp>
#include <DDS/core/media/manager.hpp>

#include <DDS/rtmp/amf0/amf0.h>
#include <DDS/rtmp/amf0/simple_buffer.h>

std::map<std::string, std::shared_ptr<rtmp_stream>> streams_;


void rtmp_session::on_close()
{
    if(streams_.count(url) > 0)
    {
        streams_[url]->leave(shared_from_this());
        if(streams_[url]->count() == 0)
            streams_.erase(url);
    }

    state = CLOSE;
    LOG(INFO) << "<rtmp> " << "closing client session.";
}

int rtmp_session::handle_media_message()
{
    if(state != MEDIA_TRANSFER)
        return -1;

    if(streams_.count(url) > 0)
    {
        streams_[url]->send(shared_from_this(), cs_in[csid_], csid_);
        cs_in[csid_]->clear();
        return 0;
    }
    else
    {
        cs_in[csid_]->clear();
        return -1;
    }
}

int rtmp_session::handle_command_message()
{
    SimpleBuffer buf;
    buf.append(reinterpret_cast<char*>(cs_in[csid_]->data()->data()), cs_in[csid_]->data()->size());
    cs_in[csid_]->clear();

    Amf0String rcommand;
    rcommand.read(&buf);

    Amf0Number tid;
    tid.read(&buf);

    if (rcommand.value == "connect" && state == CONNECT)
    {
        Amf0Object co;
        co.read(&buf);

        Amf0String* aapp = dynamic_cast<Amf0String*>(co.value_at("app"));
        app = aapp->value;

        max_chunk_size = 4096;
        send_set_chunk_size();
        send_window_ack();
        send_set_peer_bandwidth();

        Amf0String command("_result");
        Amf0Number tid(1);
        Amf0Object props;
        Amf0Object info;

        props.put("fmsVer", new Amf0String("FMS/3,0,1,123"));
        props.put("capabilities", new Amf0Number(31));

        info.put("level", new Amf0String("status"));
        info.put("code", new Amf0String("NetConnection.Connect.Success"));
        info.put("description", new Amf0String("Connection succeeded"));
        info.put("objectEncoding", new Amf0Number(0));

        SimpleBuffer buf;
        command.write(&buf);
        tid.write(&buf);
        props.write(&buf);
        info.write(&buf);

        write_chunk(csid_, 0, 0, 20, 0, reinterpret_cast<uint8_t*>(buf.data()), buf.size());

        state = CREATE_STREAM;
    }
    else if (rcommand.value == "createStream" && state == CREATE_STREAM)
    {
        Amf0String scommand("_result");
        Amf0Null sco;
        Amf0Number asid(MEDIA_STREAM_ID);

        SimpleBuffer buf;
        scommand.write(&buf);
        tid.write(&buf);
        sco.write(&buf);
        asid.write(&buf);

        write_chunk(3, 0, 0, 20, 0, reinterpret_cast<uint8_t*>(buf.data()), buf.size());

        state = PUBLISH_PLAY;
    }
    else if (rcommand.value == "releaseStream")
    {
        Amf0Null null;
        null.read(&buf);
        Amf0String acid;
        acid.read(&buf);

        cid = acid.value;
        url = app + '/' + cid;

        if(streams_.count(url) > 0)
        {
            streams_.erase(url);
        }
    }
    else if (rcommand.value == "deleteStream")
    {
        Amf0Null null;
        null.read(&buf);
        Amf0Number asid;
        asid.read(&buf);
    }
    else if (rcommand.value == "FCPublish")
    {
        Amf0Null null;
        null.read(&buf);
        Amf0String acid;
        acid.read(&buf);

        cid = acid.value;
        url = app + '/' + cid;
    }
    else if (rcommand.value == "FCUnpublish")
    {
        Amf0Null null;
        null.read(&buf);
        Amf0String acid;
        acid.read(&buf);

        if(cid == acid.value && streams_.count(url) > 0)
        {
            streams_[url]->leave(shared_from_this());
            if(streams_[url]->count() == 0)
                streams_.erase(url);
        }
    }
    else if (rcommand.value == "getStreamLength")
    {
    }
    else if (rcommand.value == "publish" && state == PUBLISH_PLAY)
    {
        Amf0Null null;
        null.read(&buf);
        Amf0String acid;
        acid.read(&buf);
        Amf0String aapp;
        aapp.read(&buf);

        app = aapp.value;
        cid = acid.value;
        url = app + '/' + cid;

        if(streams_.count(url) == 0)
        {
            streams_[url] = std::make_shared<rtmp_stream>(url);
            on_stream_create(streams_[url]->cid);
        }
        streams_[url]->join(shared_from_this(), true);

        send_stream_begin(cs_in[csid_]->stream());
        send_onStatus(0, "status", "NetStream.Publish.Start", "Start publishing");

        state = MEDIA_TRANSFER;
    }
    else if (rcommand.value == "play" && state == PUBLISH_PLAY)
    {
        Amf0Null null;
        null.read(&buf);
        Amf0String acid;
        acid.read(&buf);

        cid = acid.value;
        url = app + '/' + cid;

        if(streams_.count(url) == 0)
        {
            streams_[url] = std::make_shared<rtmp_stream>(url);
            on_stream_create(streams_[url]->cid);
        }
        streams_[url]->join(shared_from_this());
        send_stream_begin(cs_in[csid_]->stream());
        send_onStatus(0, "status", "NetStream.Play.Reset", "Reset play");
        send_onStatus(0, "status", "NetStream.Play.Start", "Start play");

        state = MEDIA_TRANSFER;
    }
    else
    {
        LOG(WARN) << "<rtmp> " << "unimplemented command type: " << rcommand.value << " or invalid state: " << state;
    }

    return 0;
}


void rtmp_session::send_onStatus(unsigned tid, std::string level, std::string code, std::string desc)
{
    Amf0String command("onStatus");
    Amf0Number atid(tid);
    Amf0Null co;
    Amf0Object info;

    info.put("level", new Amf0String(level));
    info.put("code", new Amf0String(code));
    info.put("description", new Amf0String(desc));

    SimpleBuffer buf;
    command.write(&buf);
    atid.write(&buf);
    co.write(&buf);
    info.write(&buf);

    write_chunk(3, 0, 0, 20, 0, reinterpret_cast<uint8_t*>(buf.data()), buf.size());
}
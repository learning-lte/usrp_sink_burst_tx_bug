// SPDX-License-Identifier: GPL-3.0-or-later
//

#include <uhd/utils/thread.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <boost/thread/thread.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <complex>

int UHD_SAFE_MAIN(int, char **){
    uhd::set_thread_priority_safe();

    //parameters
    std::string args="", channel_list="0";
    double seconds_in_future = 0.3;
    size_t total_num_samps=1000;
    double rate=1e6;
    float ampl=0.5;
    double freq=900.0e6;
    double gain=30;

    uhd::usrp::multi_usrp::sptr usrp = uhd::usrp::multi_usrp::make(args);

    //set usrp parameters
    usrp->set_tx_rate(rate);
    usrp->set_tx_freq(freq, 0);
    usrp->set_tx_gain(gain, 0);
    usrp->set_time_now(uhd::time_spec_t(0.0));

    //create a transmit streamer
    uhd::stream_args_t stream_args("fc32"); //complex floats
    stream_args.channels = {0};
    uhd::tx_streamer::sptr tx_stream = usrp->get_tx_stream(stream_args);

    //allocate buffer with data to send
    const size_t spb = tx_stream->get_max_num_samps();
    std::vector<std::complex<float> > buff(spb, std::complex<float>(ampl, ampl));

    //generate sine wave and store it in the buffer
    const float pi = std::acos(-1);
    const std::complex<float> ci(0, 1);
    for(unsigned int jj=0;jj<total_num_samps;jj++){
      buff[jj] = std::complex<float>((float)0.3*
                              std::exp(ci*(float)2.0*pi*(float)0.2*(float)jj));
    }
    std::vector<std::complex<float> *> buffs(1, &buff.front());

    uhd::tx_metadata_t md;
    md.start_of_burst = true;
    md.end_of_burst = false;
    md.time_spec = usrp->get_time_now() + ::uhd::time_spec_t(0.1);
    md.has_time_spec = false;
    typedef std::vector<const void *> gr_vector_const_void_star;
    tx_stream->send(gr_vector_const_void_star(1), 0, md, 1.0);

    double time_to_send = seconds_in_future;
    uhd::tx_metadata_t md2;
    md2.start_of_burst = true;
    md2.end_of_burst = true;
    md2.has_time_spec = true;
    md2.time_spec = time_to_send;
    //this 'send' call mimics what is being done in usrp_sink::start function
    //https://github.com/gnuradio/gnuradio/blob/219eae9a9c2ef7644450e71d19f8f54c12e1f9cc/gr-uhd/lib/usrp_sink_impl.cc#L611
    //this is later source underflow when first burst is sent
    tx_stream->send(buffs, total_num_samps, md2, 1.0);

    uhd::async_metadata_t async_md;
    size_t acks = 0;

    tx_stream->recv_async_msg(async_md, seconds_in_future);

    return EXIT_SUCCESS;
}

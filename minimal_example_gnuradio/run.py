#!/usr/bin/env python2

from test_grc import *
from gnuradio import uhd

class test(test_grc):
    def __init__(self):
        test_grc.__init__(self)
        self.uhd_usrp_sink_0.set_time_now(uhd.time_spec_t(0.0))

if __name__ == '__main__':
    main(top_block_cls=test)

"""
Embedded Python Blocks:

Each time this file is saved, GRC will instantiate the first class it finds
to get ports and parameters of your block. The arguments to __init__  will
be the parameters. All of them are required to have default values!
"""

#import numpy as np
#from gnuradio import gr
#import pmt


import numpy as np
from gnuradio import gr
import pmt
from time import sleep
import threading

class bursts_gen(gr.basic_block):
    def __init__(self):  # only default arguments here
        gr.basic_block.__init__(
            self,
            name='Bursts generator',
            in_sig=[],
            out_sig=[]
        )
        self.message_port_register_out(pmt.intern("bursts"))
        self.f_n = 0.05
        self.N = 1000
        self.samples = pmt.init_c32vector(self.N,np.exp(1j*np.arange(0,self.N)*2*np.pi*self.f_n))

        self.base_time = 0.1
        self.burst_spacing = 0.1
        self.sleep_before = 0.05
        self.sleep_between = self.burst_spacing
        self.threads = []
#        self.generate()

    def start(self):
        t = threading.Thread(target=self.generate)
        self.threads.append(t)
        t.start()
        return gr.basic_block.start(self)
    def stop(self):
        for t in self.threads:
            t.join()
        return gr.basic_block.stop(self)

    def generate(self):
        sleep(self.sleep_before)
        for i in xrange(0,10):
            tx_time = pmt.make_tuple(
              pmt.from_uint64(int(self.base_time + i * self.burst_spacing)),
              pmt.from_double((self.base_time + i * self.burst_spacing) % 1)
            )
            pdu_header = pmt.dict_add(pmt.make_dict(),pmt.intern('tx_time'),tx_time)
            pdu_header = pmt.PMT_NIL
            burst = pmt.cons(pdu_header,self.samples)
            self.message_port_pub(pmt.intern("bursts"), burst)
            sleep(self.sleep_between)


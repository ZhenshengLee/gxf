"""
 SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 SPDX-License-Identifier: Apache-2.0

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
"""
import unittest

from gxf.core import Graph
from gxf.std import ComputeEntity
import gxf.std as std
import gxf.sample as sample
import gxf.benchmark.benchmark_buffer_publisher as benchmark_buffer
import gxf.benchmark.benchmark_controller as benchmark_controller
import gxf.benchmark.benchmark_sink as benchmark_sink


class BlankBm(Graph):
    """ A benchmark app for benchmarking framework with no graph-under-test. """

    def __init__(self):
        super().__init__('BlankBm')

        title = 'Test Benchmark Python Graph (Blank)'

        # System components
        clock = std.set_clock(self, std.RealtimeClock(name='clock'))
        std.set_scheduler(
            self,
            std.EventBasedScheduler(name='scheduler', clock=clock, worker_thread_number=4))

        # Dummy data source
        data_source_subgraph = self.add(DummyDataSource("dummy_data_source", clock=clock))

        # Benchmark buffer and publisher
        buffer_subgraph = self.add(benchmark_buffer.BenchmarkBufferPublisher("benchmark_buffer"))

        # Benchmark sink
        sink_subgraph = self.add(benchmark_sink.BenchmarkSink(
            "benchmark_sink",
            benchmark_publisher=buffer_subgraph.get('publisher')))

        # Benchmark controller
        self.add(benchmark_controller.BenchmarkController(
            "benchmark_controller",
            clock=clock,
            title=title,
            data_source_async_scheduling_terms=[data_source_subgraph.get('async_st')],
            data_source_boolean_scheduling_terms=[data_source_subgraph.get('boolean_st')],
            benchmark_sinks=[sink_subgraph.get('sink')],
            benchmark_publishers=[buffer_subgraph.get('publisher')],
            benchmark_duration_ms=500,
            benchmark_iterations=3,
            entity_buffer_size=10))

        std.connect(src=data_source_subgraph.get('tx'), target=buffer_subgraph.get('rx'), graph=self)
        std.connect(src=buffer_subgraph.get('tx'), target=sink_subgraph.get('rx'), graph=self)


class DummyDataSource(Graph):
    """
    Dummy data source subgraph
    """

    def __init__(self, name, clock):
        super().__init__(name)
        data_source_entity = self.add(ComputeEntity("data_source", recess_period='10Hz'))
        data_source_entity.add_codelet(
            sample.PingTx(clock=clock), tx_capacity=15
        )
        data_source_entity.add(std.BooleanSchedulingTerm('boolean_st', enable_tick=True))
        data_source_entity.add(std.AsynchronousSchedulingTerm('async_st'))
        # Interface
        self.make_visible('tx', data_source_entity.signal)
        self.make_visible('boolean_st', data_source_entity.boolean_st)
        self.make_visible('async_st', data_source_entity.async_st)


class TestCore(unittest.TestCase):
    '''
    Test benchmark framework with Python graph APIs
    '''
    @classmethod
    def setUpClass(cls):
        # method will be ran once before any test is ran
        pass

    @classmethod
    def tearDownClass(cls):
        # method will be ran once after all tests have run
        pass

    def setUp(self):
        # ran before each test
        return super().setUp()

    def tearDown(self):
        # ran after each test
        return super().tearDown()

    def test_benchmark_one_source_one_sink(self):
        g = BlankBm()
        g.load_extensions()
        g.run()
        g.destroy()

if __name__ == '__main__':
    unittest.main()

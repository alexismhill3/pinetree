import unittest

from unittest.mock import MagicMock
import random
from pysinthe import feature, polymer


class TestPolymerMethods(unittest.TestCase):

    def setUp(self):
        promoter = feature.Promoter("promoter1",
                                    5,
                                    15,
                                    ["ecolipol", "rnapol"]
                                    )
        promoter.cover_signal.connect(self.fire_block)
        promoter.uncover_signal.connect(self.fire_promoter)
        terminator = feature.Terminator("myterm",
                                        50,
                                        55,
                                        {"rnapol": {"efficiency": 1.0},
                                         "ecolipol": {"efficiency": 0.6}
                                         })
        mask = feature.Mask("mask",
                            10,
                            100,
                            ["ecolipol"])

        self.pol1 = feature.Polymerase("ecolipol",
                                       10,
                                       30
                                       )
        self.pol1.start = 20
        self.pol1.stop = 30
        self.pol2 = feature.Polymerase("rnapol",
                                       10,
                                       30
                                       )
        self.pol2.start = 60
        self.pol2.stop = 70
        self.pol3 = feature.Polymerase("rnapol",
                                       10,
                                       30
                                       )
        self.pol3.start = 40
        self.pol3.stop = 50
        self.polymer = polymer.Polymer("mygenome",
                                       1,
                                       100,
                                       [promoter, terminator],
                                       mask)

        self.assertTrue(terminator.is_covered())
        self.assertFalse(terminator.was_covered())
        self.assertFalse(terminator.was_uncovered())
        self.assertTrue(promoter.is_covered())
        self.assertFalse(promoter.was_covered())
        self.assertFalse(promoter.was_uncovered())
        self.assertEqual(self.polymer.uncovered["promoter1"], 0)
        self.assertEqual(self.polymer.uncovered["myterm"], 0)
        self.fired_termination = False
        self.polymer.termination_signal.connect(self.fire_termination)
        self.promoter_fired = 0
        self.block_fired = 0

    def fire_termination(self, index, pol_name, gene_name):
        self.fired_termination = True

    def fire_promoter(self, name):
        self.promoter_fired += 1

    def fire_block(self, name):
        self.block_fired += 1

    def test_bind_polymerase(self):
        random.seed(22)
        # Promoter should be covered and inaccessible
        self.assertRaises(RuntimeError,
                          self.polymer.bind_polymerase, self.pol1, "promoter1")
        # Shift mask back 10 positions
        for i in range(10):
            self.polymer.shift_mask()
        # Bind polymerase
        self.polymer.bind_polymerase(self.pol1, "promoter1")
        # Check changes in coverings and positions
        self.assertEqual(self.pol1.start, 5)
        self.assertEqual(self.pol1.stop, 14)
        self.assertEqual(self.polymer.uncovered["promoter1"], 0)
        self.assertEqual(self.polymer.prop_sum, 30)

    def test_execute(self):
        self.setUp()
        for i in range(100):
            self.polymer.shift_mask()
        # Arrange polymerases along polymer
        self.polymer.bind_polymerase(self.pol1, "promoter1")
        for i in range(35):
            self.polymer._move_polymerase(self.pol1)
        self.polymer.bind_polymerase(self.pol2, "promoter1")
        for i in range(25):
            self.polymer._move_polymerase(self.pol2)
        self.polymer.bind_polymerase(self.pol3, "promoter1")

        for i in range(30):
            self.polymer.execute()

        # Once all of the polymerases have terminated, execute should throw
        # a RuntimeError
        with self.assertRaises(RuntimeError):
            for i in range(30):
                self.polymer.execute()

    def test_shift_mask(self):
        self.setUp()
        old_mask_start = self.polymer.mask.start
        self.polymer.shift_mask()
        self.assertEqual(old_mask_start + 1, self.polymer.mask.start)

        self.assertTrue(self.polymer.elements[0].is_covered())
        self.assertTrue(self.polymer.elements[1].is_covered())

        for i in range(10):
            self.polymer.shift_mask()

        self.assertFalse(self.polymer.elements[0].is_covered())
        self.assertTrue(self.polymer.elements[1].is_covered())

        for i in range(1000):
            self.polymer.shift_mask()
        # Make sure mask can't get shifted beyond its end position
        self.assertEqual(self.polymer.mask.start, self.polymer.mask.stop)
        self.assertFalse(self.polymer.elements[1].is_covered())

    def test_terminate(self):
        self.setUp()
        for i in range(10):
            self.polymer.shift_mask()

        self.polymer.bind_polymerase(self.pol1, "promoter1")

        old_prop_sum = self.polymer.prop_sum
        self.polymer.terminate(self.pol1, "my_gene")

        self.assertNotEqual(self.polymer.prop_sum, old_prop_sum)
        self.assertTrue(self.fired_termination)
        with self.assertRaises(ValueError):
            self.polymer.polymerases.index(self.pol1)

    def test_count_uncovered(self):
        # Check that cached count matches true count
        count = 0
        for element in self.polymer.elements:
            if element.name == "promoter1" and not element.is_covered():
                count += 1
        self.assertEqual(self.polymer.count_uncovered("promoter1"), count)

        count = 0
        for element in self.polymer.elements:
            if element.name == "myterm" and not element.is_covered():
                count += 1
        self.assertEqual(self.polymer.count_uncovered("myterm"), count)

    def test_cover_element(self):
        self.polymer.uncovered["promoter1"] = 0
        with self.assertRaises(RuntimeError):
            self.polymer.cover_element("promoter1")
        self.polymer.uncovered["promoter1"] = 5
        self.polymer.cover_element("promoter1")
        self.assertEqual(self.polymer.uncovered["promoter1"], 4)

    def test_uncover_element(self):
        self.polymer.uncovered["promoter1"] = 0
        self.polymer.uncover_element("promoter1")
        self.assertEqual(self.polymer.uncovered["promoter1"], 1)

    def test_calculate_propensity(self):
        prop_sum = 0
        for pol in self.polymer.polymerases:
            prop_sum += pol.speed
        self.assertEqual(prop_sum, self.polymer.calculate_propensity())

    def test_insert_polymerase(self):
        # Make sure we're starting with a fresh polymer
        self.setUp()

        self.polymer._insert_polymerase(self.pol2)
        self.assertEqual(self.polymer.polymerases.index(self.pol2), 0)

        self.polymer._insert_polymerase(self.pol1)
        self.assertEqual(self.polymer.polymerases.index(self.pol1), 0)
        self.assertEqual(self.polymer.polymerases.index(self.pol2), 1)

        self.polymer._insert_polymerase(self.pol3)
        self.assertEqual(self.polymer.polymerases.index(self.pol1), 0)
        self.assertEqual(self.polymer.polymerases.index(self.pol2), 2)
        self.assertEqual(self.polymer.polymerases.index(self.pol3), 1)

        # Trying to insert the same pol object twice should throw an error
        with self.assertRaises(RuntimeError):
            self.polymer._insert_polymerase(self.pol2)

    def test_choose_polymerase(self):
        # Start with clean polymer
        self.setUp()
        # There are no polymerases on the transcript so trying to randomly
        # choose a polymerase should throw an error
        with self.assertRaises(RuntimeError):
            self.polymer._choose_polymerase()
        # Move mask back
        for i in range(100):
            self.polymer.shift_mask()
        # Arrange polymerases along polymer
        self.polymer.bind_polymerase(self.pol1, "promoter1")
        for i in range(35):
            self.polymer._move_polymerase(self.pol1)
        self.polymer.bind_polymerase(self.pol2, "promoter1")
        for i in range(25):
            self.polymer._move_polymerase(self.pol2)
        self.polymer.bind_polymerase(self.pol3, "promoter1")
        # Set seed and randomly select polymerases by propensity
        random.seed(13)
        self.assertEqual(self.polymer._choose_polymerase(), self.pol3)
        self.assertEqual(self.polymer._choose_polymerase(), self.pol1)
        self.assertEqual(self.polymer._choose_polymerase(), self.pol1)

    def test_move_polymerase(self):
        # TODO: update to test for termination upon hitting end of transcript
        # Start with clean polymer
        self.setUp()
        # Shift mask back to expose promoter
        for i in range(10):
            self.polymer.shift_mask()
        self.polymer.bind_polymerase(self.pol1, "promoter1")
        self.assertEqual(self.polymer.uncovered["promoter1"], 0)
        self.assertTrue(self.polymer.elements[0].is_covered())

        # Move polymerase 10 spaces and re-expose promoter
        for i in range(11):
            self.polymer._move_polymerase(self.pol1)
        self.assertEqual(self.polymer.uncovered["promoter1"], 1)
        self.assertFalse(self.polymer.elements[0].is_covered())
        # Make sure that the mask has also moved appropriately
        self.assertEqual(self.polymer.mask.start, self.pol1.stop + 1)

        # Check collisions between polymerases
        self.polymer.bind_polymerase(self.pol2, "promoter1")
        for i in range(15):
            self.polymer._move_polymerase(self.pol2)
        # One polymerase should not be able to pass another
        self.assertEqual(self.pol2.stop + 1, self.pol1.start)
        self.assertFalse(self.polymer.elements_intersect(self.pol1, self.pol2))

        # Remove pol1
        self.polymer.terminate(self.pol1, MagicMock())
        # Make sure that pol2 is unable to shift mask back
        old_mask_start = self.polymer.mask.start
        for i in range(20):
            self.polymer._move_polymerase(self.pol2)
        self.assertEqual(old_mask_start, self.polymer.mask.start)
        self.assertEqual(self.polymer.mask.start, self.pol2.stop + 1)

        # Move back mask to expose terminator
        for i in range(60):
            self.polymer.shift_mask()
        self.assertFalse(self.polymer.elements[1].is_covered())
        self.assertTrue(self.pol2 in self.polymer.polymerases)
        # Test termination, pol should detach as soon as it hits terminator
        for i in range(25):
            self.polymer._move_polymerase(self.pol2)
        self.assertTrue(
            self.polymer.elements_intersect(self.pol2,
                                            self.polymer.elements[1])
            )
        with self.assertRaises(ValueError):
            self.polymer.polymerases.index(self.pol2)

        # Now check for readthrough
        self.polymer.bind_polymerase(self.pol1, "promoter1")
        random.seed(19)
        for i in range(42):
            self.polymer._move_polymerase(self.pol1)
        self.assertTrue(self.polymer.elements[1].readthrough)
        # Move and remove polymerase
        for i in range(20):
            self.polymer._move_polymerase(self.pol1)
        self.polymer.terminate(self.pol1, MagicMock())
        # Run polymerase through terminator again, this time it should terminate
        self.polymer.bind_polymerase(self.pol1, "promoter1")
        for i in range(35):
            self.polymer._move_polymerase(self.pol1)
        self.assertFalse(self.polymer.elements[1].readthrough)

    def test_uncover_elements(self):
        promoter1 = feature.Promoter("p1", 5, 15, ["ecolipol"])
        promoter2 = feature.Promoter("p2", 16, 20, [])
        promoter3 = feature.Promoter("p3", 21, 30, [])
        elements = [promoter1, promoter2, promoter3]
        test_polymer = polymer.Polymer("test", 1, 100, elements)

        test_pol = feature.Polymerase("ecolipol",
                                      10,
                                      30)
        test_polymer.bind_polymerase(test_pol, "p1")
        # Move polymerase manually
        test_pol.start = 12
        test_pol.stop = 12 + test_pol.footprint
        # Manually cover elements that should be covered
        promoter2.cover()
        promoter3.cover()
        # Test uncovered elements
        test_polymer._uncover_elements(test_pol)
        self.assertFalse(promoter1.is_covered())
        self.assertFalse(promoter2.is_covered())
        self.assertFalse(promoter3.is_covered())

        test_pol.start = 16
        test_pol.stop = 16 + test_pol.footprint
        promoter1.cover()
        promoter2.cover()
        promoter3.cover()
        test_polymer._uncover_elements(test_pol)
        self.assertTrue(promoter1.is_covered())
        self.assertFalse(promoter2.is_covered())
        self.assertFalse(promoter3.is_covered())

    def test_recover_elements(self):
        pass

    def test_resolve_termination(self):
        self.setUp()
        random.seed(22)
        # set up terminator
        term = feature.Terminator("myterm",
                                  23,
                                  60,
                                  {"rnapol": {"efficiency": 1.0},
                                   "ecolipol": {"efficiency": 0.6}
                                   }
                                  )
        self.pol1.start = 1
        self.pol1.stop = 23
        term.gene = "mygene"
        term.reading_frame = 1
        self.pol1.reading_frame = 1
        # Create temp termination signal
        self.pol2.release_signal.connect(lambda x: self.assertEqual(x, 60))
        # Terminator will enter readthrough
        term.readthrough = False
        self.polymer._resolve_termination(self.pol1, term)
        self.polymer._resolve_termination(self.pol1, term)
        self.assertTrue(term.readthrough)
        # Polymerase and terminator are in different reading frames
        term.readthrough = False
        self.pol2.start = 1
        self.pol2.stop = 23
        self.pol2.reading_frame = 2
        result = self.polymer._resolve_termination(self.pol2, term)
        self.assertFalse(result)

    def test_resolve_mask_collisions(self):
        self.setUp()
        for i in range(10):
            self.polymer.shift_mask()
        # This polymerase is capable of shifting mask backwards
        self.polymer.bind_polymerase(self.pol1, "promoter1")
        self.pol1.start += 6
        self.pol1.stop += 6
        old_mask_start = self.polymer.mask.start
        self.assertFalse(self.polymer._resolve_mask_collisions(self.pol1))
        self.assertEqual(self.polymer.mask.start, old_mask_start + 1)
        # The polymerase and mask should never overlap by more than 1 position
        self.pol1.start += 5
        self.pol1.stop += 5
        with self.assertRaises(RuntimeError):
            self.polymer._resolve_mask_collisions(self.pol1)

        self.setUp()
        for i in range(10):
            self.polymer.shift_mask()
        self.polymer.bind_polymerase(self.pol2, "promoter1")
        old_mask_start = self.polymer.mask.start
        old_start = self.pol2.start
        old_stop = self.pol2.stop
        self.pol2.start += 6
        self.pol2.stop += 6
        self.assertTrue(self.polymer._resolve_mask_collisions(self.pol2))
        self.assertEqual(self.pol2.start, old_start + 5)
        self.assertEqual(self.pol2.stop, old_stop + 5)
        self.assertEqual(self.polymer.mask.start, old_mask_start)

    def test_resolve_collisions(self):
        self.setUp()
        for i in range(10):
            self.polymer.shift_mask()
        self.polymer.bind_polymerase(self.pol1, "promoter1")
        for i in range(20):
            self.polymer._move_polymerase(self.pol1)
        self.polymer.bind_polymerase(self.pol2, "promoter1")
        for i in range(11):
            self.pol2.move()
        old_start = self.pol2.start
        self.assertTrue(self.polymer._resolve_collisions(self.pol2))
        self.assertEqual(old_start - 1, self.pol2.start)

        for i in range(3):
            self.pol2.move()
        with self.assertRaises(RuntimeError):
            self.polymer._resolve_collisions(self.pol2)

    def test_elements_intersect(self):
        element1 = feature.Promoter("promoter1",
                                    5,
                                    15,
                                    ["ecolipol", "rnapol"]
                                    )
        element2 = feature.Promoter("promoter1",
                                    5,
                                    15,
                                    ["ecolipol", "rnapol"]
                                    )
        self.assertTrue(self.polymer.elements_intersect(element1, element2))
        self.assertTrue(self.polymer.elements_intersect(element2, element1))

        element1.start = 15
        element1.stop = 25
        self.assertTrue(self.polymer.elements_intersect(element1, element2))
        self.assertTrue(self.polymer.elements_intersect(element2, element1))

        element1.start = 16
        self.assertFalse(self.polymer.elements_intersect(element1, element2))
        self.assertFalse(self.polymer.elements_intersect(element2, element1))

        element1.start = 3
        element1.stop = 17
        self.assertTrue(self.polymer.elements_intersect(element1, element2))
        self.assertTrue(self.polymer.elements_intersect(element2, element1))

        element1.start = 7
        element1.stop = 10
        self.assertTrue(self.polymer.elements_intersect(element1, element2))
        self.assertTrue(self.polymer.elements_intersect(element2, element1))


class TestGenomeMethods(TestPolymerMethods):

    def setUp(self):
        promoter = feature.Promoter("promoter1",
                                    5,
                                    15,
                                    ["ecolipol", "rnapol"]
                                    )
        terminator = feature.Terminator("myterm",
                                        50,
                                        55,
                                        {"rnapol": {"efficiency": 1.0},
                                         "ecolipol": {"efficiency": 0.6}
                                         })
        mask = feature.Mask("mask",
                            10,
                            700,
                            ["ecolipol"])

        self.pol1 = feature.Polymerase("ecolipol",
                                       10,
                                       30
                                       )
        self.pol1.start = 20
        self.pol1.stop = 30
        self.pol2 = feature.Polymerase("rnapol",
                                       10,
                                       30
                                       )
        self.pol2.start = 60
        self.pol2.stop = 70
        self.pol3 = feature.Polymerase("rnapol",
                                       10,
                                       30
                                       )
        self.pol3.start = 40
        self.pol3.stop = 50

        self.transcript_template = [{'type': 'transcript',
                                     'name': 'rnapol',
                                     'length': 200,
                                     'start': 10,
                                     'stop': 210,
                                     'rbs': -15},
                                    {'type': 'transcript',
                                     'name': 'proteinX',
                                     'length': 40,
                                     'start': 230,
                                     'stop': 270,
                                     'rbs': -15},
                                    {'type': 'transcript',
                                     'name': 'proteinY',
                                     'length': 300,
                                     'start': 300,
                                     'stop': 600,
                                     'rbs': -15}]

        self.polymer = polymer.Genome("mygenome",
                                      700,
                                      [promoter, terminator],
                                      self.transcript_template,
                                      mask)

        self.polymer.transcript_signal.connect(self.fire_transcript)
        self.polymer.termination_signal.connect(self.fire_termination)
        self.fired_transcript = False

    def fire_transcript(self, transcript):
        self.fired_transcript = True
        self.transcript = transcript

    def test_bind_polymerase(self):
        # TODO: Update to test that transcript is being constructed correctly
        with self.assertRaises(RuntimeError):
            self.polymer.bind_polymerase(self.pol1, "promoter1")

        for i in range(20):
            self.polymer.shift_mask()

        self.assertFalse(self.fired_transcript)
        self.polymer.bind_polymerase(self.pol1, "promoter1")
        self.assertTrue(self.fired_transcript)

        self.assertEqual(self.transcript.stop, self.polymer.stop)
        self.assertTrue(
            self.transcript.shift_mask in self.pol1.move_signal._handlers
        )

        # Test that mask in transcript gets uncovered appropriately as
        # polymerase moves
        old_mask_start = self.transcript.mask.start
        for i in range(10):
            self.polymer._move_polymerase(self.pol1)

        self.assertEqual(self.transcript.mask.start, old_mask_start + 10)

    def test_build_transcript(self):
        # TODO: update to test that overlapping genes are ordered correctly
        self.setUp()

        with self.assertRaises(AssertionError):
            self.polymer._build_transcript(0, 0)

        with self.assertRaises(RuntimeError):
            self.polymer._build_transcript(600, 600)

        transcript = self.polymer._build_transcript(0, 230)
        self.assertEqual(len(transcript.elements), 2)

        transcript = self.polymer._build_transcript(0, 300)
        self.assertEqual(len(transcript.elements), 4)

        transcript = self.polymer._build_transcript(0, 600)
        self.assertEqual(len(transcript.elements), 6)

        transcript = self.polymer._build_transcript(200, 600)
        self.assertEqual(len(transcript.elements), 4)

        self.assertEqual(transcript.mask.start, 200)
        self.assertEqual(transcript.mask.stop, 600)

        # Check positions of RBSs and tranlation stop sites
        self.assertEqual(transcript.elements[0].start, 215)
        self.assertEqual(transcript.elements[0].stop, 230)
        self.assertEqual(transcript.elements[1].start, 269)
        self.assertEqual(transcript.elements[1].stop, 270)
        self.assertEqual(transcript.elements[2].start, 285)
        self.assertEqual(transcript.elements[2].stop, 300)
        self.assertEqual(transcript.elements[3].start, 599)
        self.assertEqual(transcript.elements[3].stop, 600)


class TestTranscriptMethods(unittest.TestCase):
    """
    No transcript methods to test right now...
    """
    pass

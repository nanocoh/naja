# SPDX-FileCopyrightText: 2023 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
#
# SPDX-License-Identifier: Apache-2.0

import unittest
import naja

class SNLInstanceTest(unittest.TestCase):
  def setUp(self):
    universe = naja.NLUniverse.create()
    db = naja.NLDB.create(universe)
    lib = naja.NLLibrary.create(db)
    self.top = naja.SNLDesign.create(lib)
    self.model = naja.SNLDesign.create(lib)
    self.i0 = naja.SNLScalarTerm.create(self.model, naja.SNLTerm.Direction.Input, "I0")
    self.i1 = naja.SNLBusTerm.create(self.model, naja.SNLTerm.Direction.Input, 4, 0, "I1")
    self.o = naja.SNLScalarTerm.create(self.model, naja.SNLTerm.Direction.Output, "O")
    p0 = naja.SNLParameter.create_decimal(self.model, "REG", 34)
    p1 = naja.SNLParameter.create_binary(self.model, "INIT", 16, 0x0000)
    p2 = naja.SNLParameter.create_string(self.model, "MODE", "DEFAULT")
    p3 = naja.SNLParameter.create_boolean(self.model, "INVERTED", True)

  def tearDown(self):
    del self.top
    del self.model
    del self.i0
    del self.i1
    del self.o
    if naja.NLUniverse.get():
      naja.NLUniverse.get().destroy()

  def test0(self):
    self.assertIsNotNone(self.top)
    self.assertIsNotNone(self.model)
    self.assertFalse(any(self.top.getInstances()))
    self.assertFalse(any(self.model.getInstances()))
    ins1 = naja.SNLInstance.create(self.top, self.model, "ins1")
    self.assertEqual(0, ins1.getID())
    self.assertIsNotNone(ins1)
    self.assertEqual("ins1", ins1.getName())
    self.assertEqual(self.top, ins1.getDesign())
    self.assertEqual(self.model, ins1.getModel())
    self.assertEqual(ins1, self.top.getInstance("ins1"))
    self.assertTrue(any(self.top.getInstances()))
    self.assertEqual(1, sum(1 for d in self.top.getInstances()))
    instances = [i for i in self.top.getInstances()]
    self.assertEqual(1, len(instances))
    self.assertEqual(ins1, instances[0])
    self.assertFalse(all(False for _ in ins1.getInstTerms()))
  
  def testDestroyInstance(self):
    ins1 = naja.SNLInstance.create(self.top, self.model, "ins1")
    self.assertEqual(ins1, self.top.getInstance("ins1"))
    ins1.destroy()
    self.assertIsNone(self.top.getInstance("ins1"))

  def testRenameInstance(self):
    ins1 = naja.SNLInstance.create(self.top, self.model, "ins1")
    self.assertEqual("ins1", ins1.getName())
    ins1.setName("ins2")
    self.assertEqual("ins2", ins1.getName())
    self.assertEqual(ins1, self.top.getInstance("ins2"))
    self.assertIsNone(self.top.getInstance("ins1"))
    self.assertRaises(RuntimeError, ins1.setName, self.top) #Wrong argument type

  def testInstTerms(self):
    self.assertEqual(3, sum(1 for t in self.model.getTerms()))
    ins1 = naja.SNLInstance.create(self.top, self.model, "ins1")
    ins1copy = self.top.getInstance("ins1")
    self.assertEqual(7, sum(1 for it in ins1.getInstTerms()))
    inputs = filter(lambda it: it.getDirection() == naja.SNLTerm.Direction.Input, ins1.getInstTerms())
    outputs = filter(lambda it: it.getDirection() == naja.SNLTerm.Direction.Output, ins1.getInstTerms())
    self.assertEqual(6, sum(1 for it in inputs))
    self.assertEqual(1, sum(1 for it in outputs))
    for it in ins1.getInstTerms():
      self.assertEqual(ins1, it.getInstance())
    instTerms = tuple(ins1.getInstTerms())
    self.assertEqual(instTerms[0].getBitTerm(), self.i0)
    self.assertEqual(instTerms[1].getBitTerm(), self.i1.getBusTermBit(4))
    self.assertEqual(instTerms[2].getBitTerm(), self.i1.getBusTermBit(3))
    self.assertEqual(instTerms[3].getBitTerm(), self.i1.getBusTermBit(2))
    self.assertEqual(instTerms[4].getBitTerm(), self.i1.getBusTermBit(1))
    self.assertEqual(instTerms[5].getBitTerm(), self.i1.getBusTermBit(0))
    self.assertEqual(instTerms[6].getBitTerm(), self.o)
    with self.assertRaises(RuntimeError) as context: self.i1.getBusTermBit()
    with self.assertRaises(RuntimeError) as context: self.i1.getBusTermBit([])


  def testInstParameters(self):
    self.assertEqual(4, sum(1 for p in self.model.getParameters()))
    ins1 = naja.SNLInstance.create(self.top, self.model, "ins1")
    self.assertEqual(0, sum(1 for ip in ins1.getInstParameters()))
    p0 = self.model.getParameter("REG")
    self.assertIsNotNone(p0)
    p1 = self.model.getParameter("INIT")
    self.assertIsNotNone(p1)
    inst_p0 = naja.SNLInstParameter.create(ins1, p0, '58')
    self.assertIsNotNone(inst_p0)
    self.assertEqual(p0.getName(), inst_p0.getName())
    self.assertEqual(ins1, inst_p0.getInstance())
    inst_p1 = naja.SNLInstParameter.create(ins1, p1, '0x1234')
    self.assertEqual(p1.getName(), inst_p1.getName())
    self.assertEqual(ins1, inst_p1.getInstance())
    self.assertIsNotNone(inst_p1)
    self.assertEqual(2, sum(1 for ip in ins1.getInstParameters()))
    self.assertEqual(inst_p0, ins1.getInstParameter("REG"))
    self.assertEqual(inst_p1, ins1.getInstParameter("INIT"))
    self.assertEqual('58', inst_p0.getValue())
    self.assertEqual('0x1234', inst_p1.getValue())
    inst_p0.setValue('0x5678')
    self.assertEqual('0x5678', inst_p0.getValue())
    inst_p0.destroy()
    self.assertIsNone(ins1.getInstParameter("REG"))
    self.assertEqual(1, sum(1 for ip in ins1.getInstParameters()))
    inst_p1.destroy()
    self.assertIsNone(ins1.getInstParameter("INIT"))
    self.assertFalse(any(True for _ in ins1.getInstParameters()))

  def testInstParameterErrors(self):
    ins1 = naja.SNLInstance.create(self.top, self.model, "ins1")
    p0 = self.model.getParameter("REG")
    self.assertIsNotNone(p0)
    with self.assertRaises(RuntimeError) as context: naja.SNLInstParameter.create(ins1, ins1, p0, '58')
    with self.assertRaises(RuntimeError) as context: naja.SNLInstParameter.create(p0, p0, '58')
    with self.assertRaises(RuntimeError) as context: naja.SNLInstParameter.create(ins1, ins1, '58')
    inst_p0 = naja.SNLInstParameter.create(ins1, p0, '58')
    self.assertIsNotNone(inst_p0)
    with self.assertRaises(RuntimeError) as context: inst_p0.setValue(p0)
  
  def testDestroyInstance(self):
    ins1 = naja.SNLInstance.create(self.top, self.model, "ins1")
    self.assertEqual(ins1, self.top.getInstance("ins1"))
    ins1.destroy()
    self.assertIsNone(self.top.getInstance("ins1"))

  def testRenameInstance(self):
    ins1 = naja.SNLInstance.create(self.top, self.model, "ins1")
    self.assertEqual("ins1", ins1.getName())
    ins1.setName("ins2")
    self.assertEqual("ins2", ins1.getName())
    self.assertEqual(ins1, self.top.getInstance("ins2"))
    self.assertIsNone(self.top.getInstance("ins1"))
    self.assertRaises(RuntimeError, ins1.setName, self.top) #Wrong argument type

  def testErrors(self):
    with self.assertRaises(RuntimeError) as context: naja.SNLInstance.create(self.top, self.model, "ins1", "ERROR")
    with self.assertRaises(RuntimeError) as context: naja.SNLInstance.create("ERROR", self.model, "ins1")
    with self.assertRaises(RuntimeError) as context: naja.SNLInstance.create(self.top, "ERROR", "ins1")

    ins1 = naja.SNLInstance.create(self.top, self.model, "ins1")
    with self.assertRaises(RuntimeError) as context: ins1.getInstTerm("ERROR")
    
if __name__ == '__main__':
  unittest.main()

// SPDX-FileCopyrightText: 2023 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#include "PySNLInstance.h"

#include "SNLInstance.h"

#include "PyInterface.h"

#include "PySNLDesign.h"
#include "PySNLInstParameter.h"
#include "PySNLInstTerm.h"
#include "PySNLBitTerm.h"
#include "PySNLAttribute.h"
#include "PySNLAttributes.h"
#include "PySNLInstTerms.h"
#include "PySNLInstParameters.h"

#include "SNLDesignModeling.h"

namespace PYNAJA {

using namespace naja::NL;

#undef   ACCESS_OBJECT
#undef   ACCESS_CLASS
#define  ACCESS_OBJECT           parent_.object_
#define  ACCESS_CLASS(_pyObject)  &(_pyObject->parent_)
#define  METHOD_HEAD(function) GENERIC_METHOD_HEAD(SNLInstance, function)

static PyObject* PySNLInstance_create(PyObject*, PyObject* args) {
  PyObject* arg0 = nullptr;
  PyObject* arg1 = nullptr;
  const char* arg2 = nullptr;
  if (not PyArg_ParseTuple(args, "OO|s:SNLDB.create", &arg0, &arg1, &arg2)) {
    setError("malformed SNLInstance create method");
    return nullptr;
  }
  NLName name;
  if (arg2) {
    name = NLName(arg2);
  }

  SNLInstance* instance = nullptr;
  TRY
  if (not IsPySNLDesign(arg0)) {
    setError("SNLInstance create needs SNLDesign as first argument");
    return nullptr;
  }
  if (not IsPySNLDesign(arg1)) {
    setError("SNLInstance create needs SNLDesign as second argument");
    return nullptr;
  }
  instance = SNLInstance::create(PYSNLDesign_O(arg0), PYSNLDesign_O(arg1), name);
  NLCATCH
  return PySNLInstance_Link(instance);
}

GetObjectMethod(SNLInstance, SNLDesign, getModel)
GetObjectByName(SNLInstance, SNLInstParameter, getInstParameter)

static PyObject* PySNLDesign_getCombinatorialInputs(PySNLDesign*, PyObject* output) {
  if (IsPySNLInstTerm(output)) {
    auto outputITerm = PYSNLInstTerm_O(output);
    PySNLInstTerms* pyObjects = nullptr;
    TRY
    auto objects = new naja::NajaCollection<SNLInstTerm*>(SNLDesignModeling::getCombinatorialInputs(outputITerm));
    pyObjects = PyObject_NEW(PySNLInstTerms, &PyTypeSNLInstTerms);
    if (not pyObjects) return nullptr;
    pyObjects->object_ = objects;
    NLCATCH
    return (PyObject*)pyObjects;
  }
  setError("malformed SNLInstance.getCombinatorialInputs method");
  return nullptr;
}

static PyObject* PySNLDesign_getCombinatorialOutputs(PySNLDesign*, PyObject* input) {
  if (IsPySNLInstTerm(input)) {
    auto inputITerm = PYSNLInstTerm_O(input);
    PySNLInstTerms* pyObjects = nullptr;
    TRY
    auto objects = new naja::NajaCollection<SNLInstTerm*>(SNLDesignModeling::getCombinatorialOutputs(inputITerm));
    pyObjects = PyObject_NEW(PySNLInstTerms, &PyTypeSNLInstTerms);
    if (not pyObjects) return nullptr;
    pyObjects->object_ = objects;
    NLCATCH
    return (PyObject*)pyObjects;
  }
  setError("malformed SNLInstance.getCombinatorialOutputs method");
  return nullptr;
}

GetNameMethod(SNLInstance)

DBoLinkCreateMethod(SNLInstance)
DBoDeallocMethod(SNLInstance)

PyTypeInheritedObjectDefinitions(SNLInstance, SNLDesignObject)

static PyObject* PySNLInstance_getInstTerm(PySNLInstance* self, PyObject* args) {
  SNLInstTerm* obj = nullptr;
  METHOD_HEAD("SNLInstance.getInstTerm()")
  PySNLBitTerm* pyBitTerm = nullptr;
  if (PyArg_ParseTuple(args, "O!:SNLInstance.getInstTerm", &PyTypeSNLBitTerm, &pyBitTerm)) {
    TRY
    auto bitTerm = PYSNLBitTerm_O(pyBitTerm);
    if (bitTerm) {
      obj = selfObject->getInstTerm(bitTerm);
    }
    NLCATCH
  } else {
    setError("invalid number of parameters for getInstTerm.");
    return nullptr;
  }
  return PySNLInstTerm_Link(obj);
}

GetContainerMethod(SNLInstance, SNLInstTerm*, SNLInstTerms, InstTerms)
GetContainerMethod(SNLInstance, SNLInstParameter*, SNLInstParameters, InstParameters)
GetContainerMethod(SNLInstance, SNLAttribute, SNLAttributes, Attributes)

DirectGetIntMethod(PySNLInstance_getID, getID, PySNLInstance, SNLInstance)

static PyObject* PySNLInstance_addAttribute(PySNLInstance* self, PyObject* args) {
  METHOD_HEAD("SNLInstance.addAttribute()")
  PySNLAttribute* pyAttribute = nullptr;
  if (PyArg_ParseTuple(args, "O!", &PyTypeSNLAttribute, &pyAttribute)) {
    auto attribute = PYSNLAttribute_O(pyAttribute);
    SNLAttributes::addAttribute(selfObject, *attribute);
  } else {
    setError("invalid number of parameters for getInstTerm.");
    return nullptr;
  }
  Py_RETURN_NONE;
}

PyMethodDef PySNLInstance_Methods[] = {
  { "create", (PyCFunction)PySNLInstance_create, METH_VARARGS|METH_STATIC,
    "SNLInstance creator"},
  { "getName", (PyCFunction)PySNLInstance_getName, METH_NOARGS,
    "get SNLInstance name"},
  { "getID", (PyCFunction)PySNLInstance_getID, METH_NOARGS,
    "get the ID."},
  {"getModel", (PyCFunction)PySNLInstance_getModel, METH_NOARGS,
    "Returns the SNLInstance model SNLDesign."},
  {"getInstParameter", (PyCFunction)PySNLInstance_getInstParameter, METH_VARARGS,
    "Returns the SNLInstParameter by name."},
  {"getInstTerm", (PyCFunction)PySNLInstance_getInstTerm, METH_VARARGS,
    "Returns the SNLInstTerm corresponding to a model's SNLBitTerm."},
  {"getInstTerms", (PyCFunction)PySNLInstance_getInstTerms, METH_NOARGS,
    "get a container of SNLInstTerms."},
  {"getInstParameters", (PyCFunction)PySNLInstance_getInstParameters, METH_NOARGS,
    "get a container of SNLInstParameters."},
  {"addAttribute", (PyCFunction)PySNLInstance_addAttribute, METH_VARARGS,
    "add an attribute to the instance."},
  {"getAttributes", (PyCFunction)PySNLInstance_getAttributes, METH_NOARGS,
    "get a container of SNLAttributes."},
  { "getCombinatorialInputs", (PyCFunction)PySNLDesign_getCombinatorialInputs, METH_O|METH_STATIC,
    "get combinatorial inputs of an instance term"},
  { "getCombinatorialOutputs", (PyCFunction)PySNLDesign_getCombinatorialOutputs, METH_O|METH_STATIC,
    "get combinatorial outputs of an instance term"},
  {NULL, NULL, 0, NULL}           /* sentinel */
};

PyTypeNLFinalObjectWithNLIDLinkPyType(SNLInstance)

}
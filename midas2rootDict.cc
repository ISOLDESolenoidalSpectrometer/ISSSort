// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME midas2rootDict
#define R__NO_DEPRECATION

/*******************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define G__DICTIONARY
#include "RConfig.h"
#include "TClass.h"
#include "TDictAttributeMap.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TBuffer.h"
#include "TMemberInspector.h"
#include "TInterpreter.h"
#include "TVirtualMutex.h"
#include "TError.h"

#ifndef G__ROOT
#define G__ROOT
#endif

#include "RtypesImp.h"
#include "TIsAProxy.h"
#include "TFileMergeInfo.h"
#include <algorithm>
#include "TCollectionProxyInfo.h"
/*******************************************************************/

#include "TDataMember.h"

// The generated code does not explicitly qualifies STL entities
namespace std {} using namespace std;

// Header files passed as explicit arguments
#include "./include/Calibration.hh"
#include "./include/Common.hh"
#include "./include/CommandLineInterface.hh"
#include "./include/Converter.hh"
#include "./include/ISSEvts.hh"
#include "./include/TimeSorter.hh"
#include "./include/EventBuilder.hh"

// Header files passed via #pragma extra_include

namespace ROOT {
   static TClass *Calibration_Dictionary();
   static void Calibration_TClassManip(TClass*);
   static void delete_Calibration(void *p);
   static void deleteArray_Calibration(void *p);
   static void destruct_Calibration(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::Calibration*)
   {
      ::Calibration *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::Calibration));
      static ::ROOT::TGenericClassInfo 
         instance("Calibration", "Calibration.hh", 26,
                  typeid(::Calibration), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &Calibration_Dictionary, isa_proxy, 4,
                  sizeof(::Calibration) );
      instance.SetDelete(&delete_Calibration);
      instance.SetDeleteArray(&deleteArray_Calibration);
      instance.SetDestructor(&destruct_Calibration);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::Calibration*)
   {
      return GenerateInitInstanceLocal((::Calibration*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::Calibration*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *Calibration_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const ::Calibration*)0x0)->GetClass();
      Calibration_TClassManip(theClass);
   return theClass;
   }

   static void Calibration_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *CommandLineInterface_Dictionary();
   static void CommandLineInterface_TClassManip(TClass*);
   static void *new_CommandLineInterface(void *p = 0);
   static void *newArray_CommandLineInterface(Long_t size, void *p);
   static void delete_CommandLineInterface(void *p);
   static void deleteArray_CommandLineInterface(void *p);
   static void destruct_CommandLineInterface(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::CommandLineInterface*)
   {
      ::CommandLineInterface *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::CommandLineInterface));
      static ::ROOT::TGenericClassInfo 
         instance("CommandLineInterface", "CommandLineInterface.hh", 13,
                  typeid(::CommandLineInterface), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &CommandLineInterface_Dictionary, isa_proxy, 4,
                  sizeof(::CommandLineInterface) );
      instance.SetNew(&new_CommandLineInterface);
      instance.SetNewArray(&newArray_CommandLineInterface);
      instance.SetDelete(&delete_CommandLineInterface);
      instance.SetDeleteArray(&deleteArray_CommandLineInterface);
      instance.SetDestructor(&destruct_CommandLineInterface);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::CommandLineInterface*)
   {
      return GenerateInitInstanceLocal((::CommandLineInterface*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::CommandLineInterface*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *CommandLineInterface_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const ::CommandLineInterface*)0x0)->GetClass();
      CommandLineInterface_TClassManip(theClass);
   return theClass;
   }

   static void CommandLineInterface_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *Converter_Dictionary();
   static void Converter_TClassManip(TClass*);
   static void *new_Converter(void *p = 0);
   static void *newArray_Converter(Long_t size, void *p);
   static void delete_Converter(void *p);
   static void deleteArray_Converter(void *p);
   static void destruct_Converter(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::Converter*)
   {
      ::Converter *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::Converter));
      static ::ROOT::TGenericClassInfo 
         instance("Converter", "Converter.hh", 20,
                  typeid(::Converter), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &Converter_Dictionary, isa_proxy, 4,
                  sizeof(::Converter) );
      instance.SetNew(&new_Converter);
      instance.SetNewArray(&newArray_Converter);
      instance.SetDelete(&delete_Converter);
      instance.SetDeleteArray(&deleteArray_Converter);
      instance.SetDestructor(&destruct_Converter);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::Converter*)
   {
      return GenerateInitInstanceLocal((::Converter*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::Converter*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *Converter_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const ::Converter*)0x0)->GetClass();
      Converter_TClassManip(theClass);
   return theClass;
   }

   static void Converter_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static void *new_ISSEvts(void *p = 0);
   static void *newArray_ISSEvts(Long_t size, void *p);
   static void delete_ISSEvts(void *p);
   static void deleteArray_ISSEvts(void *p);
   static void destruct_ISSEvts(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::ISSEvts*)
   {
      ::ISSEvts *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::ISSEvts >(0);
      static ::ROOT::TGenericClassInfo 
         instance("ISSEvts", ::ISSEvts::Class_Version(), "ISSEvts.hh", 12,
                  typeid(::ISSEvts), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::ISSEvts::Dictionary, isa_proxy, 4,
                  sizeof(::ISSEvts) );
      instance.SetNew(&new_ISSEvts);
      instance.SetNewArray(&newArray_ISSEvts);
      instance.SetDelete(&delete_ISSEvts);
      instance.SetDeleteArray(&deleteArray_ISSEvts);
      instance.SetDestructor(&destruct_ISSEvts);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::ISSEvts*)
   {
      return GenerateInitInstanceLocal((::ISSEvts*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::ISSEvts*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static TClass *TimeSorter_Dictionary();
   static void TimeSorter_TClassManip(TClass*);
   static void delete_TimeSorter(void *p);
   static void deleteArray_TimeSorter(void *p);
   static void destruct_TimeSorter(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::TimeSorter*)
   {
      ::TimeSorter *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::TimeSorter));
      static ::ROOT::TGenericClassInfo 
         instance("TimeSorter", "TimeSorter.hh", 26,
                  typeid(::TimeSorter), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &TimeSorter_Dictionary, isa_proxy, 4,
                  sizeof(::TimeSorter) );
      instance.SetDelete(&delete_TimeSorter);
      instance.SetDeleteArray(&deleteArray_TimeSorter);
      instance.SetDestructor(&destruct_TimeSorter);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::TimeSorter*)
   {
      return GenerateInitInstanceLocal((::TimeSorter*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::TimeSorter*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *TimeSorter_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const ::TimeSorter*)0x0)->GetClass();
      TimeSorter_TClassManip(theClass);
   return theClass;
   }

   static void TimeSorter_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *EventBuilder_Dictionary();
   static void EventBuilder_TClassManip(TClass*);
   static void delete_EventBuilder(void *p);
   static void deleteArray_EventBuilder(void *p);
   static void destruct_EventBuilder(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::EventBuilder*)
   {
      ::EventBuilder *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::EventBuilder));
      static ::ROOT::TGenericClassInfo 
         instance("EventBuilder", "EventBuilder.hh", 34,
                  typeid(::EventBuilder), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &EventBuilder_Dictionary, isa_proxy, 4,
                  sizeof(::EventBuilder) );
      instance.SetDelete(&delete_EventBuilder);
      instance.SetDeleteArray(&deleteArray_EventBuilder);
      instance.SetDestructor(&destruct_EventBuilder);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::EventBuilder*)
   {
      return GenerateInitInstanceLocal((::EventBuilder*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::EventBuilder*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *EventBuilder_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const ::EventBuilder*)0x0)->GetClass();
      EventBuilder_TClassManip(theClass);
   return theClass;
   }

   static void EventBuilder_TClassManip(TClass* ){
   }

} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr ISSEvts::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *ISSEvts::Class_Name()
{
   return "ISSEvts";
}

//______________________________________________________________________________
const char *ISSEvts::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::ISSEvts*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int ISSEvts::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::ISSEvts*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *ISSEvts::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::ISSEvts*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *ISSEvts::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::ISSEvts*)0x0)->GetClass(); }
   return fgIsA;
}

namespace ROOT {
   // Wrapper around operator delete
   static void delete_Calibration(void *p) {
      delete ((::Calibration*)p);
   }
   static void deleteArray_Calibration(void *p) {
      delete [] ((::Calibration*)p);
   }
   static void destruct_Calibration(void *p) {
      typedef ::Calibration current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::Calibration

namespace ROOT {
   // Wrappers around operator new
   static void *new_CommandLineInterface(void *p) {
      return  p ? new(p) ::CommandLineInterface : new ::CommandLineInterface;
   }
   static void *newArray_CommandLineInterface(Long_t nElements, void *p) {
      return p ? new(p) ::CommandLineInterface[nElements] : new ::CommandLineInterface[nElements];
   }
   // Wrapper around operator delete
   static void delete_CommandLineInterface(void *p) {
      delete ((::CommandLineInterface*)p);
   }
   static void deleteArray_CommandLineInterface(void *p) {
      delete [] ((::CommandLineInterface*)p);
   }
   static void destruct_CommandLineInterface(void *p) {
      typedef ::CommandLineInterface current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::CommandLineInterface

namespace ROOT {
   // Wrappers around operator new
   static void *new_Converter(void *p) {
      return  p ? new(p) ::Converter : new ::Converter;
   }
   static void *newArray_Converter(Long_t nElements, void *p) {
      return p ? new(p) ::Converter[nElements] : new ::Converter[nElements];
   }
   // Wrapper around operator delete
   static void delete_Converter(void *p) {
      delete ((::Converter*)p);
   }
   static void deleteArray_Converter(void *p) {
      delete [] ((::Converter*)p);
   }
   static void destruct_Converter(void *p) {
      typedef ::Converter current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::Converter

//______________________________________________________________________________
void ISSEvts::Streamer(TBuffer &R__b)
{
   // Stream an object of class ISSEvts.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(ISSEvts::Class(),this);
   } else {
      R__b.WriteClassBuffer(ISSEvts::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_ISSEvts(void *p) {
      return  p ? new(p) ::ISSEvts : new ::ISSEvts;
   }
   static void *newArray_ISSEvts(Long_t nElements, void *p) {
      return p ? new(p) ::ISSEvts[nElements] : new ::ISSEvts[nElements];
   }
   // Wrapper around operator delete
   static void delete_ISSEvts(void *p) {
      delete ((::ISSEvts*)p);
   }
   static void deleteArray_ISSEvts(void *p) {
      delete [] ((::ISSEvts*)p);
   }
   static void destruct_ISSEvts(void *p) {
      typedef ::ISSEvts current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::ISSEvts

namespace ROOT {
   // Wrapper around operator delete
   static void delete_TimeSorter(void *p) {
      delete ((::TimeSorter*)p);
   }
   static void deleteArray_TimeSorter(void *p) {
      delete [] ((::TimeSorter*)p);
   }
   static void destruct_TimeSorter(void *p) {
      typedef ::TimeSorter current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::TimeSorter

namespace ROOT {
   // Wrapper around operator delete
   static void delete_EventBuilder(void *p) {
      delete ((::EventBuilder*)p);
   }
   static void deleteArray_EventBuilder(void *p) {
      delete [] ((::EventBuilder*)p);
   }
   static void destruct_EventBuilder(void *p) {
      typedef ::EventBuilder current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::EventBuilder

namespace ROOT {
   static TClass *vectorlEvoidmUgR_Dictionary();
   static void vectorlEvoidmUgR_TClassManip(TClass*);
   static void *new_vectorlEvoidmUgR(void *p = 0);
   static void *newArray_vectorlEvoidmUgR(Long_t size, void *p);
   static void delete_vectorlEvoidmUgR(void *p);
   static void deleteArray_vectorlEvoidmUgR(void *p);
   static void destruct_vectorlEvoidmUgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<void*>*)
   {
      vector<void*> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<void*>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<void*>", -2, "vector", 469,
                  typeid(vector<void*>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEvoidmUgR_Dictionary, isa_proxy, 0,
                  sizeof(vector<void*>) );
      instance.SetNew(&new_vectorlEvoidmUgR);
      instance.SetNewArray(&newArray_vectorlEvoidmUgR);
      instance.SetDelete(&delete_vectorlEvoidmUgR);
      instance.SetDeleteArray(&deleteArray_vectorlEvoidmUgR);
      instance.SetDestructor(&destruct_vectorlEvoidmUgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<void*> >()));

      ::ROOT::AddClassAlternate("vector<void*>","std::__1::vector<void*, std::__1::allocator<void*> >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<void*>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEvoidmUgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<void*>*)0x0)->GetClass();
      vectorlEvoidmUgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEvoidmUgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEvoidmUgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<void*> : new vector<void*>;
   }
   static void *newArray_vectorlEvoidmUgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<void*>[nElements] : new vector<void*>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEvoidmUgR(void *p) {
      delete ((vector<void*>*)p);
   }
   static void deleteArray_vectorlEvoidmUgR(void *p) {
      delete [] ((vector<void*>*)p);
   }
   static void destruct_vectorlEvoidmUgR(void *p) {
      typedef vector<void*> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<void*>

namespace ROOT {
   static TClass *vectorlEvectorlEvectorlEfloatgRsPgRsPgR_Dictionary();
   static void vectorlEvectorlEvectorlEfloatgRsPgRsPgR_TClassManip(TClass*);
   static void *new_vectorlEvectorlEvectorlEfloatgRsPgRsPgR(void *p = 0);
   static void *newArray_vectorlEvectorlEvectorlEfloatgRsPgRsPgR(Long_t size, void *p);
   static void delete_vectorlEvectorlEvectorlEfloatgRsPgRsPgR(void *p);
   static void deleteArray_vectorlEvectorlEvectorlEfloatgRsPgRsPgR(void *p);
   static void destruct_vectorlEvectorlEvectorlEfloatgRsPgRsPgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<vector<vector<float> > >*)
   {
      vector<vector<vector<float> > > *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<vector<vector<float> > >));
      static ::ROOT::TGenericClassInfo 
         instance("vector<vector<vector<float> > >", -2, "vector", 469,
                  typeid(vector<vector<vector<float> > >), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEvectorlEvectorlEfloatgRsPgRsPgR_Dictionary, isa_proxy, 0,
                  sizeof(vector<vector<vector<float> > >) );
      instance.SetNew(&new_vectorlEvectorlEvectorlEfloatgRsPgRsPgR);
      instance.SetNewArray(&newArray_vectorlEvectorlEvectorlEfloatgRsPgRsPgR);
      instance.SetDelete(&delete_vectorlEvectorlEvectorlEfloatgRsPgRsPgR);
      instance.SetDeleteArray(&deleteArray_vectorlEvectorlEvectorlEfloatgRsPgRsPgR);
      instance.SetDestructor(&destruct_vectorlEvectorlEvectorlEfloatgRsPgRsPgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<vector<vector<float> > > >()));

      ::ROOT::AddClassAlternate("vector<vector<vector<float> > >","std::__1::vector<std::__1::vector<std::__1::vector<float, std::__1::allocator<float> >, std::__1::allocator<std::__1::vector<float, std::__1::allocator<float> > > >, std::__1::allocator<std::__1::vector<std::__1::vector<float, std::__1::allocator<float> >, std::__1::allocator<std::__1::vector<float, std::__1::allocator<float> > > > > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<vector<vector<float> > >*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEvectorlEvectorlEfloatgRsPgRsPgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<vector<vector<float> > >*)0x0)->GetClass();
      vectorlEvectorlEvectorlEfloatgRsPgRsPgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEvectorlEvectorlEfloatgRsPgRsPgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEvectorlEvectorlEfloatgRsPgRsPgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<vector<vector<float> > > : new vector<vector<vector<float> > >;
   }
   static void *newArray_vectorlEvectorlEvectorlEfloatgRsPgRsPgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<vector<vector<float> > >[nElements] : new vector<vector<vector<float> > >[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEvectorlEvectorlEfloatgRsPgRsPgR(void *p) {
      delete ((vector<vector<vector<float> > >*)p);
   }
   static void deleteArray_vectorlEvectorlEvectorlEfloatgRsPgRsPgR(void *p) {
      delete [] ((vector<vector<vector<float> > >*)p);
   }
   static void destruct_vectorlEvectorlEvectorlEfloatgRsPgRsPgR(void *p) {
      typedef vector<vector<vector<float> > > current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<vector<vector<float> > >

namespace ROOT {
   static TClass *vectorlEvectorlElonggRsPgR_Dictionary();
   static void vectorlEvectorlElonggRsPgR_TClassManip(TClass*);
   static void *new_vectorlEvectorlElonggRsPgR(void *p = 0);
   static void *newArray_vectorlEvectorlElonggRsPgR(Long_t size, void *p);
   static void delete_vectorlEvectorlElonggRsPgR(void *p);
   static void deleteArray_vectorlEvectorlElonggRsPgR(void *p);
   static void destruct_vectorlEvectorlElonggRsPgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<vector<long> >*)
   {
      vector<vector<long> > *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<vector<long> >));
      static ::ROOT::TGenericClassInfo 
         instance("vector<vector<long> >", -2, "vector", 469,
                  typeid(vector<vector<long> >), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEvectorlElonggRsPgR_Dictionary, isa_proxy, 0,
                  sizeof(vector<vector<long> >) );
      instance.SetNew(&new_vectorlEvectorlElonggRsPgR);
      instance.SetNewArray(&newArray_vectorlEvectorlElonggRsPgR);
      instance.SetDelete(&delete_vectorlEvectorlElonggRsPgR);
      instance.SetDeleteArray(&deleteArray_vectorlEvectorlElonggRsPgR);
      instance.SetDestructor(&destruct_vectorlEvectorlElonggRsPgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<vector<long> > >()));

      ::ROOT::AddClassAlternate("vector<vector<long> >","std::__1::vector<std::__1::vector<long, std::__1::allocator<long> >, std::__1::allocator<std::__1::vector<long, std::__1::allocator<long> > > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<vector<long> >*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEvectorlElonggRsPgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<vector<long> >*)0x0)->GetClass();
      vectorlEvectorlElonggRsPgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEvectorlElonggRsPgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEvectorlElonggRsPgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<vector<long> > : new vector<vector<long> >;
   }
   static void *newArray_vectorlEvectorlElonggRsPgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<vector<long> >[nElements] : new vector<vector<long> >[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEvectorlElonggRsPgR(void *p) {
      delete ((vector<vector<long> >*)p);
   }
   static void deleteArray_vectorlEvectorlElonggRsPgR(void *p) {
      delete [] ((vector<vector<long> >*)p);
   }
   static void destruct_vectorlEvectorlElonggRsPgR(void *p) {
      typedef vector<vector<long> > current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<vector<long> >

namespace ROOT {
   static TClass *vectorlEvectorlEfloatgRsPgR_Dictionary();
   static void vectorlEvectorlEfloatgRsPgR_TClassManip(TClass*);
   static void *new_vectorlEvectorlEfloatgRsPgR(void *p = 0);
   static void *newArray_vectorlEvectorlEfloatgRsPgR(Long_t size, void *p);
   static void delete_vectorlEvectorlEfloatgRsPgR(void *p);
   static void deleteArray_vectorlEvectorlEfloatgRsPgR(void *p);
   static void destruct_vectorlEvectorlEfloatgRsPgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<vector<float> >*)
   {
      vector<vector<float> > *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<vector<float> >));
      static ::ROOT::TGenericClassInfo 
         instance("vector<vector<float> >", -2, "vector", 469,
                  typeid(vector<vector<float> >), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEvectorlEfloatgRsPgR_Dictionary, isa_proxy, 0,
                  sizeof(vector<vector<float> >) );
      instance.SetNew(&new_vectorlEvectorlEfloatgRsPgR);
      instance.SetNewArray(&newArray_vectorlEvectorlEfloatgRsPgR);
      instance.SetDelete(&delete_vectorlEvectorlEfloatgRsPgR);
      instance.SetDeleteArray(&deleteArray_vectorlEvectorlEfloatgRsPgR);
      instance.SetDestructor(&destruct_vectorlEvectorlEfloatgRsPgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<vector<float> > >()));

      ::ROOT::AddClassAlternate("vector<vector<float> >","std::__1::vector<std::__1::vector<float, std::__1::allocator<float> >, std::__1::allocator<std::__1::vector<float, std::__1::allocator<float> > > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<vector<float> >*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEvectorlEfloatgRsPgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<vector<float> >*)0x0)->GetClass();
      vectorlEvectorlEfloatgRsPgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEvectorlEfloatgRsPgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEvectorlEfloatgRsPgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<vector<float> > : new vector<vector<float> >;
   }
   static void *newArray_vectorlEvectorlEfloatgRsPgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<vector<float> >[nElements] : new vector<vector<float> >[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEvectorlEfloatgRsPgR(void *p) {
      delete ((vector<vector<float> >*)p);
   }
   static void deleteArray_vectorlEvectorlEfloatgRsPgR(void *p) {
      delete [] ((vector<vector<float> >*)p);
   }
   static void destruct_vectorlEvectorlEfloatgRsPgR(void *p) {
      typedef vector<vector<float> > current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<vector<float> >

namespace ROOT {
   static TClass *vectorlEvectorlETVector2gRsPgR_Dictionary();
   static void vectorlEvectorlETVector2gRsPgR_TClassManip(TClass*);
   static void *new_vectorlEvectorlETVector2gRsPgR(void *p = 0);
   static void *newArray_vectorlEvectorlETVector2gRsPgR(Long_t size, void *p);
   static void delete_vectorlEvectorlETVector2gRsPgR(void *p);
   static void deleteArray_vectorlEvectorlETVector2gRsPgR(void *p);
   static void destruct_vectorlEvectorlETVector2gRsPgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<vector<TVector2> >*)
   {
      vector<vector<TVector2> > *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<vector<TVector2> >));
      static ::ROOT::TGenericClassInfo 
         instance("vector<vector<TVector2> >", -2, "vector", 469,
                  typeid(vector<vector<TVector2> >), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEvectorlETVector2gRsPgR_Dictionary, isa_proxy, 0,
                  sizeof(vector<vector<TVector2> >) );
      instance.SetNew(&new_vectorlEvectorlETVector2gRsPgR);
      instance.SetNewArray(&newArray_vectorlEvectorlETVector2gRsPgR);
      instance.SetDelete(&delete_vectorlEvectorlETVector2gRsPgR);
      instance.SetDeleteArray(&deleteArray_vectorlEvectorlETVector2gRsPgR);
      instance.SetDestructor(&destruct_vectorlEvectorlETVector2gRsPgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<vector<TVector2> > >()));

      ::ROOT::AddClassAlternate("vector<vector<TVector2> >","std::__1::vector<std::__1::vector<TVector2, std::__1::allocator<TVector2> >, std::__1::allocator<std::__1::vector<TVector2, std::__1::allocator<TVector2> > > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<vector<TVector2> >*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEvectorlETVector2gRsPgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<vector<TVector2> >*)0x0)->GetClass();
      vectorlEvectorlETVector2gRsPgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEvectorlETVector2gRsPgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEvectorlETVector2gRsPgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<vector<TVector2> > : new vector<vector<TVector2> >;
   }
   static void *newArray_vectorlEvectorlETVector2gRsPgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<vector<TVector2> >[nElements] : new vector<vector<TVector2> >[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEvectorlETVector2gRsPgR(void *p) {
      delete ((vector<vector<TVector2> >*)p);
   }
   static void deleteArray_vectorlEvectorlETVector2gRsPgR(void *p) {
      delete [] ((vector<vector<TVector2> >*)p);
   }
   static void destruct_vectorlEvectorlETVector2gRsPgR(void *p) {
      typedef vector<vector<TVector2> > current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<vector<TVector2> >

namespace ROOT {
   static TClass *vectorlEstringgR_Dictionary();
   static void vectorlEstringgR_TClassManip(TClass*);
   static void *new_vectorlEstringgR(void *p = 0);
   static void *newArray_vectorlEstringgR(Long_t size, void *p);
   static void delete_vectorlEstringgR(void *p);
   static void deleteArray_vectorlEstringgR(void *p);
   static void destruct_vectorlEstringgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<string>*)
   {
      vector<string> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<string>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<string>", -2, "vector", 469,
                  typeid(vector<string>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEstringgR_Dictionary, isa_proxy, 0,
                  sizeof(vector<string>) );
      instance.SetNew(&new_vectorlEstringgR);
      instance.SetNewArray(&newArray_vectorlEstringgR);
      instance.SetDelete(&delete_vectorlEstringgR);
      instance.SetDeleteArray(&deleteArray_vectorlEstringgR);
      instance.SetDestructor(&destruct_vectorlEstringgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<string> >()));

      ::ROOT::AddClassAlternate("vector<string>","std::__1::vector<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >, std::__1::allocator<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<string>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEstringgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<string>*)0x0)->GetClass();
      vectorlEstringgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEstringgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEstringgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<string> : new vector<string>;
   }
   static void *newArray_vectorlEstringgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<string>[nElements] : new vector<string>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEstringgR(void *p) {
      delete ((vector<string>*)p);
   }
   static void deleteArray_vectorlEstringgR(void *p) {
      delete [] ((vector<string>*)p);
   }
   static void destruct_vectorlEstringgR(void *p) {
      typedef vector<string> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<string>

namespace ROOT {
   static TClass *vectorlElonggR_Dictionary();
   static void vectorlElonggR_TClassManip(TClass*);
   static void *new_vectorlElonggR(void *p = 0);
   static void *newArray_vectorlElonggR(Long_t size, void *p);
   static void delete_vectorlElonggR(void *p);
   static void deleteArray_vectorlElonggR(void *p);
   static void destruct_vectorlElonggR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<long>*)
   {
      vector<long> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<long>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<long>", -2, "vector", 469,
                  typeid(vector<long>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlElonggR_Dictionary, isa_proxy, 0,
                  sizeof(vector<long>) );
      instance.SetNew(&new_vectorlElonggR);
      instance.SetNewArray(&newArray_vectorlElonggR);
      instance.SetDelete(&delete_vectorlElonggR);
      instance.SetDeleteArray(&deleteArray_vectorlElonggR);
      instance.SetDestructor(&destruct_vectorlElonggR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<long> >()));

      ::ROOT::AddClassAlternate("vector<long>","std::__1::vector<long, std::__1::allocator<long> >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<long>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlElonggR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<long>*)0x0)->GetClass();
      vectorlElonggR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlElonggR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlElonggR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<long> : new vector<long>;
   }
   static void *newArray_vectorlElonggR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<long>[nElements] : new vector<long>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlElonggR(void *p) {
      delete ((vector<long>*)p);
   }
   static void deleteArray_vectorlElonggR(void *p) {
      delete [] ((vector<long>*)p);
   }
   static void destruct_vectorlElonggR(void *p) {
      typedef vector<long> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<long>

namespace ROOT {
   static TClass *vectorlEintgR_Dictionary();
   static void vectorlEintgR_TClassManip(TClass*);
   static void *new_vectorlEintgR(void *p = 0);
   static void *newArray_vectorlEintgR(Long_t size, void *p);
   static void delete_vectorlEintgR(void *p);
   static void deleteArray_vectorlEintgR(void *p);
   static void destruct_vectorlEintgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<int>*)
   {
      vector<int> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<int>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<int>", -2, "vector", 469,
                  typeid(vector<int>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEintgR_Dictionary, isa_proxy, 0,
                  sizeof(vector<int>) );
      instance.SetNew(&new_vectorlEintgR);
      instance.SetNewArray(&newArray_vectorlEintgR);
      instance.SetDelete(&delete_vectorlEintgR);
      instance.SetDeleteArray(&deleteArray_vectorlEintgR);
      instance.SetDestructor(&destruct_vectorlEintgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<int> >()));

      ::ROOT::AddClassAlternate("vector<int>","std::__1::vector<int, std::__1::allocator<int> >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<int>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEintgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<int>*)0x0)->GetClass();
      vectorlEintgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEintgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEintgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<int> : new vector<int>;
   }
   static void *newArray_vectorlEintgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<int>[nElements] : new vector<int>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEintgR(void *p) {
      delete ((vector<int>*)p);
   }
   static void deleteArray_vectorlEintgR(void *p) {
      delete [] ((vector<int>*)p);
   }
   static void destruct_vectorlEintgR(void *p) {
      typedef vector<int> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<int>

namespace ROOT {
   static TClass *vectorlEfloatgR_Dictionary();
   static void vectorlEfloatgR_TClassManip(TClass*);
   static void *new_vectorlEfloatgR(void *p = 0);
   static void *newArray_vectorlEfloatgR(Long_t size, void *p);
   static void delete_vectorlEfloatgR(void *p);
   static void deleteArray_vectorlEfloatgR(void *p);
   static void destruct_vectorlEfloatgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<float>*)
   {
      vector<float> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<float>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<float>", -2, "vector", 469,
                  typeid(vector<float>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEfloatgR_Dictionary, isa_proxy, 0,
                  sizeof(vector<float>) );
      instance.SetNew(&new_vectorlEfloatgR);
      instance.SetNewArray(&newArray_vectorlEfloatgR);
      instance.SetDelete(&delete_vectorlEfloatgR);
      instance.SetDeleteArray(&deleteArray_vectorlEfloatgR);
      instance.SetDestructor(&destruct_vectorlEfloatgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<float> >()));

      ::ROOT::AddClassAlternate("vector<float>","std::__1::vector<float, std::__1::allocator<float> >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<float>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEfloatgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<float>*)0x0)->GetClass();
      vectorlEfloatgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEfloatgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEfloatgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<float> : new vector<float>;
   }
   static void *newArray_vectorlEfloatgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<float>[nElements] : new vector<float>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEfloatgR(void *p) {
      delete ((vector<float>*)p);
   }
   static void deleteArray_vectorlEfloatgR(void *p) {
      delete [] ((vector<float>*)p);
   }
   static void destruct_vectorlEfloatgR(void *p) {
      typedef vector<float> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<float>

namespace ROOT {
   static TClass *vectorlEdoublegR_Dictionary();
   static void vectorlEdoublegR_TClassManip(TClass*);
   static void *new_vectorlEdoublegR(void *p = 0);
   static void *newArray_vectorlEdoublegR(Long_t size, void *p);
   static void delete_vectorlEdoublegR(void *p);
   static void deleteArray_vectorlEdoublegR(void *p);
   static void destruct_vectorlEdoublegR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<double>*)
   {
      vector<double> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<double>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<double>", -2, "vector", 469,
                  typeid(vector<double>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEdoublegR_Dictionary, isa_proxy, 0,
                  sizeof(vector<double>) );
      instance.SetNew(&new_vectorlEdoublegR);
      instance.SetNewArray(&newArray_vectorlEdoublegR);
      instance.SetDelete(&delete_vectorlEdoublegR);
      instance.SetDeleteArray(&deleteArray_vectorlEdoublegR);
      instance.SetDestructor(&destruct_vectorlEdoublegR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<double> >()));

      ::ROOT::AddClassAlternate("vector<double>","std::__1::vector<double, std::__1::allocator<double> >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<double>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEdoublegR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<double>*)0x0)->GetClass();
      vectorlEdoublegR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEdoublegR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEdoublegR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<double> : new vector<double>;
   }
   static void *newArray_vectorlEdoublegR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<double>[nElements] : new vector<double>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEdoublegR(void *p) {
      delete ((vector<double>*)p);
   }
   static void deleteArray_vectorlEdoublegR(void *p) {
      delete [] ((vector<double>*)p);
   }
   static void destruct_vectorlEdoublegR(void *p) {
      typedef vector<double> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<double>

namespace ROOT {
   static TClass *vectorlETVector2gR_Dictionary();
   static void vectorlETVector2gR_TClassManip(TClass*);
   static void *new_vectorlETVector2gR(void *p = 0);
   static void *newArray_vectorlETVector2gR(Long_t size, void *p);
   static void delete_vectorlETVector2gR(void *p);
   static void deleteArray_vectorlETVector2gR(void *p);
   static void destruct_vectorlETVector2gR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<TVector2>*)
   {
      vector<TVector2> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<TVector2>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<TVector2>", -2, "vector", 469,
                  typeid(vector<TVector2>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlETVector2gR_Dictionary, isa_proxy, 0,
                  sizeof(vector<TVector2>) );
      instance.SetNew(&new_vectorlETVector2gR);
      instance.SetNewArray(&newArray_vectorlETVector2gR);
      instance.SetDelete(&delete_vectorlETVector2gR);
      instance.SetDeleteArray(&deleteArray_vectorlETVector2gR);
      instance.SetDestructor(&destruct_vectorlETVector2gR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<TVector2> >()));

      ::ROOT::AddClassAlternate("vector<TVector2>","std::__1::vector<TVector2, std::__1::allocator<TVector2> >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<TVector2>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlETVector2gR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<TVector2>*)0x0)->GetClass();
      vectorlETVector2gR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlETVector2gR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlETVector2gR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<TVector2> : new vector<TVector2>;
   }
   static void *newArray_vectorlETVector2gR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<TVector2>[nElements] : new vector<TVector2>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlETVector2gR(void *p) {
      delete ((vector<TVector2>*)p);
   }
   static void deleteArray_vectorlETVector2gR(void *p) {
      delete [] ((vector<TVector2>*)p);
   }
   static void destruct_vectorlETVector2gR(void *p) {
      typedef vector<TVector2> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<TVector2>

namespace {
  void TriggerDictionaryInitialization_midas2rootDict_Impl() {
    static const char* headers[] = {
"./include/Calibration.hh",
"./include/Common.hh",
"./include/CommandLineInterface.hh",
"./include/Converter.hh",
"./include/ISSEvts.hh",
"./include/TimeSorter.hh",
"./include/EventBuilder.hh",
0
    };
    static const char* includePaths[] = {
"./include",
"/opt/local/libexec/root6/include/root",
"/Users/lpgaff/cernbox/Research/ISS/DetectorTests/Midas-to-Root-ISS/",
0
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "midas2rootDict dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_AutoLoading_Map;
class __attribute__((annotate("$clingAutoload$./include/Calibration.hh")))  Calibration;
class __attribute__((annotate("$clingAutoload$./include/CommandLineInterface.hh")))  CommandLineInterface;
class __attribute__((annotate("$clingAutoload$./include/Converter.hh")))  Converter;
class __attribute__((annotate("$clingAutoload$./include/ISSEvts.hh")))  ISSEvts;
class __attribute__((annotate("$clingAutoload$./include/TimeSorter.hh")))  TimeSorter;
class __attribute__((annotate("$clingAutoload$./include/EventBuilder.hh")))  EventBuilder;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "midas2rootDict dictionary payload"


#define _BACKWARD_BACKWARD_WARNING_H
// Inline headers
#include "./include/Calibration.hh"
#include "./include/Common.hh"
#include "./include/CommandLineInterface.hh"
#include "./include/Converter.hh"
#include "./include/ISSEvts.hh"
#include "./include/TimeSorter.hh"
#include "./include/EventBuilder.hh"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[] = {
"Calibration", payloadCode, "@",
"CommandLineInterface", payloadCode, "@",
"Converter", payloadCode, "@",
"EventBuilder", payloadCode, "@",
"ISSEvts", payloadCode, "@",
"TimeSorter", payloadCode, "@",
nullptr
};
    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("midas2rootDict",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_midas2rootDict_Impl, {}, classesHeaders, /*hasCxxModule*/false);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_midas2rootDict_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_midas2rootDict() {
  TriggerDictionaryInitialization_midas2rootDict_Impl();
}

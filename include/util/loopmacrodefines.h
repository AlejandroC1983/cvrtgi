/*
Copyright 2022 Alejandro Cosin & Gustavo Patow

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef _LOOPMACRODEFINES_H_
#define _LOOPMACRODEFINES_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/commonnamespace.h"

// CLASS FORWARDING

// NAMESPACE
using namespace commonnamespace;

// DEFINES

/// Taken from http://stackoverflow.com/questions/24392000/c-define-a-for-loop-macro

/// For loop
#define forA(counter) for(uint a = 0; a < counter; ++a)
#define forB(counter) for(uint b = 0; b < counter; ++b)
#define forC(counter) for(uint c = 0; c < counter; ++c)
#define forD(counter) for(uint d = 0; d < counter; ++d)
#define forE(counter) for(uint e = 0; e < counter; ++e)
#define forF(counter) for(uint f = 0; f < counter; ++f)
#define forG(counter) for(uint g = 0; g < counter; ++g)
#define forH(counter) for(uint h = 0; h < counter; ++h)
#define forI(counter) for(uint i = 0; i < counter; ++i)
#define forJ(counter) for(uint j = 0; j < counter; ++j)
#define forK(counter) for(uint k = 0; k < counter; ++k)
#define forL(counter) for(uint l = 0; l < counter; ++l)
#define forM(counter) for(uint m = 0; m < counter; ++m)
#define forN(counter) for(uint n = 0; n < counter; ++n)
#define forO(counter) for(uint o = 0; o < counter; ++o)
#define forP(counter) for(uint p = 0; p < counter; ++p)
#define forQ(counter) for(uint q = 0; q < counter; ++q)
#define forR(counter) for(uint r = 0; r < counter; ++r)
#define forS(counter) for(uint s = 0; s < counter; ++s)
#define forT(counter) for(uint t = 0; t < counter; ++t)

/// For loop starting from the initial value given as second parameter
#define forAFrom(start, counter) for(uint a = start; a < counter; ++a)
#define forBFrom(start, counter) for(uint b = start; b < counter; ++b)
#define forCFrom(start, counter) for(uint c = start; c < counter; ++c)
#define forDFrom(start, counter) for(uint d = start; d < counter; ++d)
#define forEFrom(start, counter) for(uint e = start; e < counter; ++e)
#define forFFrom(start, counter) for(uint f = start; f < counter; ++f)
#define forGFrom(start, counter) for(uint g = start; g < counter; ++g)
#define forHFrom(start, counter) for(uint h = start; h < counter; ++h)
#define forIFrom(start, counter) for(uint i = start; i < counter; ++i)
#define forJFrom(start, counter) for(uint j = start; j < counter; ++j)
#define forKFrom(start, counter) for(uint k = start; k < counter; ++k)
#define forLFrom(start, counter) for(uint l = start; l < counter; ++l)
#define forMFrom(start, counter) for(uint m = start; m < counter; ++m)
#define forNFrom(start, counter) for(uint n = start; n < counter; ++n)
#define forOFrom(start, counter) for(uint o = start; o < counter; ++o)
#define forPFrom(start, counter) for(uint p = start; p < counter; ++p)
#define forQFrom(start, counter) for(uint q = start; q < counter; ++q)
#define forRFrom(start, counter) for(uint r = start; r < counter; ++r)
#define forSFrom(start, counter) for(uint s = start; s < counter; ++s)
#define forTFrom(start, counter) for(uint t = start; t < counter; ++t)

/// For loop with a step given as parameter
#define forAStep(step, counter) for(uint a = 0; a < counter; a += step)
#define forBStep(step, counter) for(uint b = 0; b < counter; b += step)
#define forCStep(step, counter) for(uint c = 0; c < counter; c += step)
#define forDStep(step, counter) for(uint d = 0; d < counter; d += step)
#define forEStep(step, counter) for(uint e = 0; e < counter; e += step)
#define forFStep(step, counter) for(uint f = 0; f < counter; f += step)
#define forGStep(step, counter) for(uint g = 0; g < counter; g += step)
#define forHStep(step, counter) for(uint h = 0; h < counter; h += step)
#define forIStep(step, counter) for(uint i = 0; i < counter; i += step)
#define forJStep(step, counter) for(uint j = 0; j < counter; j += step)
#define forKStep(step, counter) for(uint k = 0; k < counter; k += step)
#define forLStep(step, counter) for(uint l = 0; l < counter; l += step)
#define forMStep(step, counter) for(uint m = 0; m < counter; m += step)
#define forNStep(step, counter) for(uint n = 0; n < counter; n += step)
#define forOStep(step, counter) for(uint o = 0; o < counter; o += step)
#define forPStep(step, counter) for(uint p = 0; p < counter; p += step)
#define forQStep(step, counter) for(uint q = 0; q < counter; q += step)
#define forRStep(step, counter) for(uint r = 0; r < counter; r += step)
#define forSStep(step, counter) for(uint s = 0; s < counter; s += step)
#define forTStep(step, counter) for(uint t = 0; t < counter; t += step)

/// Taken from http://stackoverflow.com/questions/8164643/getting-the-right-value-type
/// and http://stackoverflow.com/questions/17499417/why-does-scope-resolution-fail-in-presence-of-decltype

/// Foreach loop
#define foreachA(myVector) typedef decltype(myVector) myType; for(myType::value_type &a : myVector)
#define foreachB(myVector) typedef decltype(myVector) myType; for(myType::value_type &b : myVector)
#define foreachC(myVector) typedef decltype(myVector) myType; for(myType::value_type &c : myVector)
#define foreachD(myVector) typedef decltype(myVector) myType; for(myType::value_type &d : myVector)
#define foreachE(myVector) typedef decltype(myVector) myType; for(myType::value_type &e : myVector)
#define foreachF(myVector) typedef decltype(myVector) myType; for(myType::value_type &f : myVector)
#define foreachG(myVector) typedef decltype(myVector) myType; for(myType::value_type &g : myVector)
#define foreachH(myVector) typedef decltype(myVector) myType; for(myType::value_type &h : myVector)
#define foreachI(myVector) typedef decltype(myVector) myType; for(myType::value_type &i : myVector)
#define foreachJ(myVector) typedef decltype(myVector) myType; for(myType::value_type &j : myVector)
#define foreachK(myVector) typedef decltype(myVector) myType; for(myType::value_type &k : myVector)
#define foreachL(myVector) typedef decltype(myVector) myType; for(myType::value_type &l : myVector)
#define foreachM(myVector) typedef decltype(myVector) myType; for(myType::value_type &m : myVector)
#define foreachN(myVector) typedef decltype(myVector) myType; for(myType::value_type &n : myVector)
#define foreachO(myVector) typedef decltype(myVector) myType; for(myType::value_type &o : myVector)
#define foreachP(myVector) typedef decltype(myVector) myType; for(myType::value_type &p : myVector)
#define foreachQ(myVector) typedef decltype(myVector) myType; for(myType::value_type &q : myVector)
#define foreachR(myVector) typedef decltype(myVector) myType; for(myType::value_type &r : myVector)
#define foreachS(myVector) typedef decltype(myVector) myType; for(myType::value_type &s : myVector)
#define foreachT(myVector) typedef decltype(myVector) myType; for(myType::value_type &t : myVector)

// For iteration
#define forAT(myVector) for(auto at = myVector.begin(); at != myVector.end(); ++at)
#define forBT(myVector) for(auto bt = myVector.begin(); bt != myVector.end(); ++bt)
#define forCT(myVector) for(auto ct = myVector.begin(); ct != myVector.end(); ++ct)
#define forDT(myVector) for(auto dt = myVector.begin(); dt != myVector.end(); ++dt)
#define forET(myVector) for(auto et = myVector.begin(); et != myVector.end(); ++et)
#define forFT(myVector) for(auto ft = myVector.begin(); ft != myVector.end(); ++ft)
#define forGT(myVector) for(auto gt = myVector.begin(); gt != myVector.end(); ++gt)
#define forHT(myVector) for(auto ht = myVector.begin(); ht != myVector.end(); ++ht)
#define forIT(myVector) for(auto it = myVector.begin(); it != myVector.end(); ++it)
#define forJT(myVector) for(auto jt = myVector.begin(); jt != myVector.end(); ++jt)
#define forKT(myVector) for(auto kt = myVector.begin(); kt != myVector.end(); ++kt)
#define forLT(myVector) for(auto lt = myVector.begin(); lt != myVector.end(); ++lt)
#define forMT(myVector) for(auto mt = myVector.begin(); mt != myVector.end(); ++mt)
#define forNT(myVector) for(auto nt = myVector.begin(); nt != myVector.end(); ++nt)
#define forOT(myVector) for(auto ot = myVector.begin(); ot != myVector.end(); ++ot)
#define forPT(myVector) for(auto pt = myVector.begin(); pt != myVector.end(); ++pt)
#define forQT(myVector) for(auto qt = myVector.begin(); qt != myVector.end(); ++qt)
#define forRT(myVector) for(auto rt = myVector.begin(); rt != myVector.end(); ++rt)
#define forST(myVector) for(auto st = myVector.begin(); st != myVector.end(); ++st)
#define forTT(myVector) for(auto tt = myVector.begin(); tt != myVector.end(); ++tt)

// DEFINES

#endif _LOOPMACRODEFINES_H_

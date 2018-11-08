/*
    This file is part of PajeNG

    PajeNG is free software: you can redistribute it and/or modify
    it under the terms of the GNU Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PajeNG is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Public License for more details.

    You should have received a copy of the GNU Public License
    along with PajeNG. If not, see <http://www.gnu.org/licenses/>.
*/
#include <sys/time.h>
#include "PajeUnity.h"
#include "PajeException.h"
#include "PajeFileReader.h"
#include "PajeFlexReader.h"
#include "PajeProbabilisticSimulator.h"

extern int _pajeng_user_defined;
extern int _pajeng_out_of_core;
extern int _pajeng_entity_hierarchy_dump;
extern int _pajeng_quiet;

static double gettime (void)
{
  struct timeval tr;
  gettimeofday(&tr, NULL);
  return (double)tr.tv_sec+(double)tr.tv_usec/1000000;
}

PajeUnity::PajeUnity (bool flexReader,
		      bool strictHeader,
		      std::string tracefilename,
		      double stopat,
		      bool ignoreIncompleteLinks,
		      char *probabilistic,
		      bool noImbrication,
		      bool userDefined,
		      bool outOfCore,
		      bool entityDump,
		      bool quiet)
{
  //userDefined dump
  _pajeng_user_defined = userDefined;

  //out of core simulation
  _pajeng_out_of_core = outOfCore;

  //entity hierarchy dump
  _pajeng_entity_hierarchy_dump = entityDump;

  //out of core simulation
  _pajeng_quiet = quiet;

  //basic configuration
  this->flexReader = flexReader;

  //the global PajeDefinitions object
  definitions = new PajeDefinitions (strictHeader);
 
  //alloc reader
  if (flexReader){
    if (tracefilename.empty()){
      reader = new PajeFlexReader(definitions);
    }else{
      reader = new PajeFlexReader(tracefilename, definitions);
    }
  }else{
    if (tracefilename.empty()){
      reader = new PajeFileReader();
    }else{
      reader = new PajeFileReader (tracefilename);
    }
  }

  //alloc decoder and simulator
  if (!flexReader){
    decoder = new PajeEventDecoder(definitions);
  }
  if (probabilistic){
    simulator = new PajeProbabilisticSimulator (probabilistic);
  }else{
    simulator = new PajeSimulator (stopat, ignoreIncompleteLinks, noImbrication);
  }


  //connect components
  if (flexReader){
    reader->setOutputComponent (simulator);
    simulator->setInputComponent (reader);
  }else{
    reader->setOutputComponent (decoder);
    decoder->setInputComponent (reader);
    decoder->setOutputComponent (simulator);
    simulator->setInputComponent (decoder);
  }
  simulator->setOutputComponent (this);
  this->setInputComponent (simulator);

  //read and simulate
  t1 = gettime();
  reader->startReading ();
  while (reader->hasMoreData() && simulator->keepSimulating()){
    reader->readNextChunk ();
  }
  reader->finishedReading ();
  t2 = gettime();
}

PajeUnity::~PajeUnity ()
{
  reader->closeReading();
  delete reader;
  if (!flexReader){
    delete decoder;
  }
  delete simulator;
  delete definitions;
}

double PajeUnity::getTime ()
{
  return t2-t1;
}

void PajeUnity::report ()
{
  simulator->report();
}

void PajeUnity::reportDot ()
{
  simulator->reportDotFormat();
}

void PajeUnity::reportContainer ()
{
  simulator->reportContainer();
}

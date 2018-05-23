/**
* Copyright (C) 2017 Chalmers Revere
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
* USA.
*/

#include <iostream>
#include "cluon-complete.hpp"
#include "logic-brakes.hpp"

#define PI 3.14159265359f



Brakes::Brakes(bool verbose, uint32_t id, cluon::OD4Session &od4, unit32_t senderStamp)
  : m_od4(od4)
  , m_senderStamp(senderStamp)
{
  setUp();
  (void)verbose;
  (void)id;
}

Brakes::~Brakes()
{
  tearDown();
}


void Brakes::nextContainer(cluon::data::Envelope &a_container)
{
  std::cout << "I recieved a container!" << std::endl;
  if (a_container.dataType() == opendlv::proxy::GroundDecelerationRequest::ID()) {
     auto deceleration = cluon::extractMessage<opendlv::proxy::GroundDecelerationRequest>(std::move(a_container));
     float pwm = 3.5f * deceleration.groundDeceleration();
     uint32_t pwmrequest = static_cast<uint32_t>(pwm);

     opendlv::proxy::PulseWidthModulationRequest pr;
     pr.dutyCycleNs(pwmrequest);

     std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
     cluon::data::TimeStamp sampleTime = cluon::time::convert(tp);
     m_od4.send(pr,sampleTime,m_senderStamp); // Last argument is sender stamp

      // opendlv::proxy::SwitchStateRequest state;
      // if (pwm < 0) {
      //   state.setState(1);
      //  } else {
      //   state.setState(0);
      // }
      // odcore::data::Container c2(state);
      // c2.setSenderStamp(m_stateID);
      // getConference().send(c2);
  }
}

void Brakes::setUp()
{
  // std::string const exampleConfig =
  std::cout << "Setting up brakes" << std::endl;
}

void Brakes::tearDown()
{
}

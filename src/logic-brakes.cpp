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



Brakes::Brakes(bool verbose, uint32_t id, cluon::OD4Session &od4)
  : m_od4(od4)
  , m_latestMessage()
{
  setUp(verbose, id);
}

Brakes::~Brakes()
{
  tearDown();
}


void Brakes::nextContainer(cluon::data::Envelope &a_container)
{
  if (a_container.dataType() == opendlv::proxy::GroundDecelerationRequest::ID()) {
    if (cluon::time::toMicroseconds(a_container.sampleTimeStamp()) > cluon::time::toMicroseconds(m_latestMessage)) {
      m_latestMessage = a_container.sampleTimeStamp();
      auto GroundDecelerationRequest = cluon::extractMessage<opendlv::proxy::GroundDecelerationRequest>(std::move(a_container));
      float deceleration = GroundDecelerationRequest.groundDeceleration();

      float dutyCycle = deceleration2dutyCycleNs(deceleration);
      uint32_t dutyCycleNs = static_cast<uint32_t>(std::abs(static_cast<int32_t>(std::round(dutyCycle))));
      dutyCycleNs = (dutyCycleNs < 50000) ? dutyCycleNs : 50000;

      if (dutyCycleNs>5000){
        opendlv::proxy::PulseWidthModulationRequest pr;
        pr.dutyCycleNs(dutyCycleNs);

        std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
        cluon::data::TimeStamp sampleTime = cluon::time::convert(tp);
        m_od4.send(pr,sampleTime,m_senderStamp);
      }
    }
  }

  if (a_container.dataType() == opendlv::proxy::GroundAccelerationRequest::ID()){
    if (cluon::time::toMicroseconds(a_container.sampleTimeStamp()) > cluon::time::toMicroseconds(m_latestMessage)) {
      m_latestMessage = a_container.sampleTimeStamp();
      auto GroundAccelerationRequest = cluon::extractMessage<opendlv::proxy::GroundAccelerationRequest>(std::move(a_container));
      float acceleration = GroundAccelerationRequest.groundAcceleration();

      if (acceleration > 0) {
        opendlv::proxy::PulseWidthModulationRequest pr;
        pr.dutyCycleNs(0);

        std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
        cluon::data::TimeStamp sampleTime = cluon::time::convert(tp);
        m_od4.send(pr,sampleTime,m_senderStamp);
      }
    }
  }
}

float Brakes::deceleration2dutyCycleNs(float deceleration){
  float dutyCycleNs = deceleration/13.32f*50000.0f;
  return dutyCycleNs;
}

float Brakes::calcHydraulic2Pwm(float hydraulicPressure){
  float dutyCycleNs;
  float airPressure = 0.0f*std::pow(hydraulicPressure,3.0f)+0.0f*std::pow(hydraulicPressure,2.0f)+213.4286f*hydraulicPressure-177.0f;
  dutyCycleNs = (airPressure/7.0f*50000.0f);
  return dutyCycleNs;
}

void Brakes::setUp(bool verbose, u_int32_t id)
{
  if (verbose){
    std::cout << "Setting up steering" << std::endl;
    std::cout << "Steering ID: " << id << std::endl;
  }
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
  m_latestMessage = cluon::time::convert(tp);
}

void Brakes::tearDown()
{
}

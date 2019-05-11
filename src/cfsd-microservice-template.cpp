/*
 * Copyright (C) 2018  Naichen Wang
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "cluon-complete.hpp"
#include "opendlv-standard-message-set.hpp"
#include "cfsd-extended-message-set.hpp"
#include <cstdint>
#include <iostream>

int32_t main(int32_t argc, char **argv) {
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
    if (0 == commandlineArguments.count("cid")) {
        std::cerr << argv[0] << "template for message exchange between microservices." << std::endl;
        std::cerr << "Usage:   " << argv[0] << " --cid=<OD4 session> [--verbose]" << std::endl;
        std::cerr << "         --cid:    CID of the OD4Session to send and receive messages" << std::endl;
        std::cerr << "Example: " << argv[0] << " --cid=131 --verbose" << std::endl;
    }
    else {
        const bool VERBOSE{commandlineArguments.count("verbose") != 0};
        uint16_t cid = static_cast<uint16_t>(std::stoi(commandlineArguments["cid"]));
        cluon::OD4Session od4{cid};
        std::cerr <<  "Start conversation at Opendlv session cid: "<< cid << std::endl;

        
        auto SwitchStateReading = [VERBOSE](cluon::data::Envelope &&env){
            opendlv::proxy::SwitchStateReading p = cluon::extractMessage<opendlv::proxy::SwitchStateReading>(std::move(env));
            if(env.senderStamp() == 1406){
                if (VERBOSE){
                    std::cout << "1900: msg: " << p.state()<< std::endl;
                }
            }else if (env.senderStamp() == 1091){
                if (VERBOSE){
                    std::cout << "1901: msg: " << p.state()<< std::endl;
                }
            }
        };
        od4.dataTrigger(opendlv::proxy::SwitchStateReading::ID(), SwitchStateReading);

        uint32_t counter{0};
        auto ping = [&od4, &counter](){
            // Define a message...
            {
                cluon::data::TimeStamp now{cluon::time::now()};
                opendlv::proxy::SwitchStateRequest msg;
                msg.state(counter);
                od4.send(msg, now, 1902);
            }
            {
                cluon::data::TimeStamp now{cluon::time::now()};
                opendlv::cfsdProxy::TorqueRequestDual msg;
                msg.torqueLeft(1000);
                msg.torqueRight(1000);
                od4.send(msg, now, 1902);
            }

            return true;
        };
        // Finally, register the lambda as time-triggered function.
        const float RUN_AT_TWO_HERTZ{2.0f};
        od4.timeTrigger(RUN_AT_TWO_HERTZ, ping); // Won't return until stopped.
        
    }
    return 0;

}
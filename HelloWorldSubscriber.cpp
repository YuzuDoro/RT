// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file HelloWorldSubscriber.cpp
 *
 */

#include "HelloWorldSubscriber.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/Domain.h>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <string>
#include<stdio.h>
#include<sys/time.h>
#include<unistd.h>
//*********************************************************
//#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
//#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
//using namespace eprosima::fastdds::dds;
#include <ctime>
#include<fstream>
#include<iostream>
//*********************************************************
long long maxdif=0;
long long mindif=99999999999;
double avgdif=0;
int number=0;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastdds::rtps;
using namespace std;
ofstream myout("out.txt");
HelloWorldSubscriber::HelloWorldSubscriber()
    : mp_participant(nullptr)
    , mp_subscriber(nullptr)
{
}

bool HelloWorldSubscriber::init()
{
    ParticipantAttributes PParam;
    PParam.rtps.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol_t::SIMPLE;
    PParam.rtps.builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
    PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    PParam.rtps.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    PParam.rtps.setName("Participant_sub");

    // SharedMem transport configuration
    PParam.rtps.useBuiltinTransports = false;

    auto sm_transport = std::make_shared<SharedMemTransportDescriptor>();
    sm_transport->segment_size(2 * 1024 * 1024);
    PParam.rtps.userTransports.push_back(sm_transport);

    // UDP
    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();
    //udp_transport->interfaceWhiteList.push_back("127.0.0.1");
    PParam.rtps.userTransports.push_back(udp_transport);

    mp_participant = Domain::createParticipant(PParam);
    if (mp_participant == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE


    Domain::registerType(mp_participant, &m_type);
    //CREATE THE SUBSCRIBER
    SubscriberAttributes Rparam;
    Rparam.topic.topicKind = NO_KEY;
    Rparam.topic.topicDataType = "HelloWorld";
    Rparam.topic.topicName = "HelloWorldSharedMemTopic";
    Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Rparam.topic.historyQos.depth = 30;
    Rparam.topic.resourceLimitsQos.max_samples = 50;
    Rparam.topic.resourceLimitsQos.allocated_samples = 20;
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;

    mp_subscriber = Domain::createSubscriber(mp_participant, Rparam, (SubscriberListener*)&m_listener);

    if (mp_subscriber == nullptr)
    {
        return false;
    }


    return true;
}

HelloWorldSubscriber::~HelloWorldSubscriber()
{
    // TODO Auto-generated destructor stub
    Domain::removeParticipant(mp_participant);
}

void HelloWorldSubscriber::SubListener::onSubscriptionMatched(
        Subscriber* /*sub*/,
        MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        n_matched++;
        std::cout << "Subscriber matched" << std::endl;
        //ofstream myout("out.txt",ios::trunc);
        myout.open("out.txt",ios::trunc);//clear contents
        myout.close();
    }
    else
    {
    	myout.open("out.txt",ios::app);
    	myout<<"Max:"<<maxdif<<std::endl;
    	myout<<"Min:"<<mindif<<std::endl;
    	myout<<"Avg:"<<avgdif/number<<std::endl;
    	myout.close();
        n_matched--;
        std::cout << "Subscriber unmatched" << std::endl;
    }
}

void HelloWorldSubscriber::SubListener::onNewDataMessage(
        Subscriber* sub)
{
    if (sub->takeNextData((void*)m_Hello.get(), &m_info))
    {
        if (m_info.sampleKind == ALIVE)
        {
            this->n_samples++;
            const size_t data_size = m_Hello->data().size();
            // Print your structure data here.
            timeval rawtime2;
	    gettimeofday(&rawtime2,NULL);
	    //std::string tim = std::to_string(rawtime2.tv_sec*1000000+rawtime2.tv_usec);
	    long long tim2=rawtime2.tv_sec%1000000*1000000+rawtime2.tv_usec;
	    long long tim1=0;
	    for(int i=data_size-13;i<data_size-1;i++){
	    	tim1=tim1*10+(m_Hello->data()[i]-'0'); 
	    }
	    long long difference=tim2-tim1;
            std::cout << "Message " << m_Hello->message() << " " << m_Hello->index()
                      << " RECEIVED With " << data_size << "(bytes) of Data. "
                 //std::cout << (char*)&m_Hello->data()[data_size - 17]
                //(char*)&m_Hello->data()[0]
                      << std::endl;
            //ofstream myout("out.txt",ios::app);
            myout.open("out.txt",ios::app);
            myout<<difference<<std::endl;
            avgdif+=difference;
            number++;
            if(difference > maxdif){
	    	maxdif=difference;
	    }
	    if(difference < mindif){
	    	mindif=difference;
	    }
	    //myout<<"Max:"<<maxdif<<std::endl;
	    myout.close();
            //std::cout << tim1 <<std::endl;
            //std::cout << tim2 <<std::endl;
            //std::cout << difference <<std::endl;
                 
        }
    }
}

void HelloWorldSubscriber::run()
{
    std::cout << "Subscriber running. Please press enter to stop the Subscriber" << std::endl;
    std::cin.ignore();
}

void HelloWorldSubscriber::run(
        uint32_t number)
{
    std::cout << "Subscriber running until " << number << "samples have been received" << std::endl;
    while (number > this->m_listener.n_samples)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

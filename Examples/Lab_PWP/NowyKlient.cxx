/*=========================================================================

  Program:   OpenIGTLink -- Example for Tracker Client Program
  Language:  C++

  Copyright (c) Insight Software Consortium. All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <iostream>
#include <math.h>
#include <cstdlib>

#include "igtlOSUtil.h"
#include "igtlPointMessage.h"
#include "igtlClientSocket.h"
#include "igtlServerSocket.h"


int ReceiveTransform(igtl::Socket* socket, igtl::MessageHeader* header);
int ReceivePosition(igtl::Socket* socket, igtl::MessageHeader* header);
int ReceiveImage(igtl::Socket* socket, igtl::MessageHeader* header);
int ReceiveStatus(igtl::Socket* socket, igtl::MessageHeader* header);

#if OpenIGTLink_PROTOCOL_VERSION >= 2
int ReceivePoint(igtl::Socket* socket, igtl::MessageHeader* header);
int ReceiveTrajectory(igtl::Socket* socket, igtl::MessageHeader::Pointer& header);
int ReceiveString(igtl::Socket* socket, igtl::MessageHeader* header);
int ReceiveBind(igtl::Socket* socket, igtl::MessageHeader* header);
int ReceiveCapability(igtl::Socket* socket, igtl::MessageHeader* header);
#endif //OpenIGTLink_PROTOCOL_VERSION >= 2


int main(int argc, char* argv[])
{
  //------------------------------------------------------------
  // Parse Arguments

  if (argc != 3) // check number of arguments
    {
    // If not correct, print usage
    std::cerr << "Usage: " << argv[0] << " <hostname> <port> <fps>"    << std::endl;
    std::cerr << "    <hostname> : IP or host name"                    << std::endl;
    std::cerr << "    <port>     : Port # (18944 in Slicer default)"   << std::endl;
    exit(0);
    }

  char*  hostname = argv[1];
  int    port     = atoi(argv[2]);

  //------------------------------------------------------------
  // Establish Connection

  igtl::ClientSocket::Pointer socket;
  socket = igtl::ClientSocket::New();
  int r = socket->ConnectToServer(hostname, port);

  if (r != 0)
    {
    std::cerr << "Cannot connect to the server." << std::endl;
    exit(0);
    }

  //------------------------------------------------------------
  // Allocate Transform Message Class

  igtl::PointMessage::Pointer pointMsg;
  pointMsg = igtl::PointMessage::New();
  pointMsg->SetDeviceName("PointSender");

  //---------------------------
  // Create 1st point
  igtl::PointElement::Pointer point0;
  point0 = igtl::PointElement::New();
  point0->SetName("POINT_0");
  point0->SetGroupName("GROUP_0");
  point0->SetRGBA(0xFF, 0x00, 0x00, 0xFF);
  point0->SetPosition(10.0, 20.0, 30.0);
  point0->SetRadius(15.0);
  point0->SetOwner("IMAGE_0");
  
  //---------------------------
  // Create 2nd point
  igtl::PointElement::Pointer point1;
  point1 = igtl::PointElement::New();
  point1->SetName("POINT_1");
  point1->SetGroupName("GROUP_0");
  point1->SetRGBA(0x00, 0xFF, 0x00, 0xFF);
  point1->SetPosition(40.0, 50.0, 60.0);
  point1->SetRadius(45.0);
  point1->SetOwner("IMAGE_0");
  
  //---------------------------
  // Create 3rd point
  igtl::PointElement::Pointer point2;
  point2 = igtl::PointElement::New();
  point2->SetName("POINT_2");
  point2->SetGroupName("GROUP_0");
  point2->SetRGBA(0x00, 0x00, 0xFF, 0xFF);
  point2->SetPosition(70.0, 80.0, 90.0);
  point2->SetRadius(75.0);
  point2->SetOwner("IMAGE_0");
  
  //---------------------------
  // Pack into the point message
  pointMsg->AddPointElement(point0);
  pointMsg->AddPointElement(point1);
  pointMsg->AddPointElement(point2);
  pointMsg->Pack();
  
  //------------------------------------------------------------
  // Send
  socket->Send(pointMsg->GetPackPointer(), pointMsg->GetPackSize());
  

  std::cerr << " Name      : " <<  std::endl;



  while (1)
  {

      if (socket.IsNotNull())
      {

          // Create a message buffer to receive header
          igtl::MessageHeader::Pointer headerMsg;
          headerMsg = igtl::MessageHeader::New();

          //------------------------------------------------------------
          // Allocate a time stamp 
          igtl::TimeStamp::Pointer ts;
          ts = igtl::TimeStamp::New();

          //------------------------------------------------------------
          // loop
          for (int i = 0; i < 100; i++)
          {

              // Initialize receive buffer
              headerMsg->InitPack();

              // Receive generic header from the socket
              bool timeout(false);
              igtlUint64 g = socket->Receive(headerMsg->GetPackPointer(), headerMsg->GetPackSize(), timeout);

              if (g == 0)
              {

                  socket->CloseSocket();
              }
              if (g != headerMsg->GetPackSize())
              {
                  continue;
              }

              // Deserialize the header
              headerMsg->Unpack();

              // Get time stamp
              igtlUint32 sec;
              igtlUint32 nanosec;

              headerMsg->GetTimeStamp(ts);
              ts->GetTimeStamp(&sec, &nanosec);

              std::cerr << "Time stamp: "
                  << nanosec << std::endl;


              if (strcmp(headerMsg->GetDeviceType(), "POINT") == 0)
              {
                  ReceivePoint(socket, headerMsg);

              }

              //------------------------------------------------------------
              // Close the socket
              socket->CloseSocket();

          }
      }

  }
  }
      
int ReceivePoint(igtl::Socket* socket, igtl::MessageHeader* header)
{
    

    std::cerr << "Receiving POINT data type." << std::endl;

    // Create a message buffer to receive transform data
    igtl::PointMessage::Pointer pointMsg;
    pointMsg = igtl::PointMessage::New();
    pointMsg->SetMessageHeader(header);
    pointMsg->AllocatePack();

    // Receive transform data from the socket
    bool timeout(false);
    socket->Receive(pointMsg->GetPackBodyPointer(), pointMsg->GetPackBodySize(), timeout);

    // Deserialize the transform data
    // If you want to skip CRC check, call Unpack() without argument.
    int c = pointMsg->Unpack(1);

    if (c & igtl::MessageHeader::UNPACK_BODY) // if CRC check is OK
    {
        int nElements = pointMsg->GetNumberOfPointElement();
        for (int i = 0; i < nElements; i++)
        {
            igtl::PointElement::Pointer pointElement;
            pointMsg->GetPointElement(i, pointElement);

            igtlUint8 rgba[4];
            pointElement->GetRGBA(rgba);

            igtlFloat32 pos[3];
            pointElement->GetPosition(pos);

            std::cerr << "========== Element #" << i << " ==========" << std::endl;
            std::cerr << " Position  : ( " << std::fixed << pos[0] << ", " << pos[1] << ", " << pos[2] << " )" << std::endl;
            std::cerr << "================================" << std::endl;
        }
    }

    return 1;
}




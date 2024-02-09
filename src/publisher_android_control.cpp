// Copyright 2016 Open Source Robotics Foundation, Inc.
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

#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"


#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>


using namespace std;
// mspm

using namespace std::chrono_literals;

/* This example creates a subclass of Node and uses std::bind() to register a
 * member function as a callback from the timer. */

#include "rclcpp/rclcpp.hpp"
 
int main(int argc, char *argv[]) {

  bool is_running = true;
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("android_publisher");
  auto message = std_msgs::msg::String();
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  publisher_ = node->create_publisher<std_msgs::msg::String>("gui", 10);
  

  int listening = socket(AF_INET, SOCK_STREAM, 0);

  if (listening == -1)
  {
      cerr << "Can't create a socket! Quitting" << endl;
      return -1;
  }

  int opt = 1;
  // Set SO_REUSEADDR on a socket to true (1): This allow connecting also to connect during TIME_WAIT
  if (setsockopt(listening, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    cerr << "Can't set socket options! Quitting" << endl;
    return -1;
  }

  // Bind the ip address and port to a socket
  sockaddr_in hint;
  hint.sin_family = AF_INET;
  hint.sin_port = htons(6000);
  inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr); // for any address

  if (bind(listening, (sockaddr*)&hint, sizeof(hint)) == -1) {
    RCLCPP_INFO(node->get_logger(), "Can't bind");
    return -2;
  };

  // Tell Winsock the socket is for listening
  listen(listening, SOMAXCONN);


while (is_running) {

  // Wait for a connection
    sockaddr_in client;
    socklen_t clientSize = sizeof(client);
    RCLCPP_INFO(node->get_logger(), "Wait TCP socket connection");
    int clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
    RCLCPP_INFO(node->get_logger(), "TCP socket connected");
    if (clientSocket == -1) {
      cerr << "Can't accept connection" << endl;
      return -1;
    }


    // Convert client's IP address from binary to text
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client.sin_addr, clientIP, INET_ADDRSTRLEN);
    cout << "Verify - Connection from " << clientIP << " requsted." << endl;

    // Check if the client's IP matches the desired IP
    if (strcmp(clientIP, "10.242.57.245") != 0) {
      // IP address does not match, close the connection
      close(clientSocket);
      cout << "Connection from " << clientIP << " rejected." << endl;
    } else {
      // IP address matches, proceed with handling the connection
      cout << "Connection from Pixel 7 IP:" << clientIP << " accepted." << endl;
      // Handle the connection, e.g., by reading or writing data
    }  

    // Connection has been established, now send a message to the client
    const char* welcomeMsg = "Robot online\n";
    int sendResult = send(clientSocket, welcomeMsg, strlen(welcomeMsg) + 1, 0);
    if (sendResult == -1) {
      cerr << "Failed to send message to client" << endl;
      // Handle error, e.g., by closing the socket
    } else {
      cout << "Message sent to client: " << welcomeMsg << endl;
    }






    char host[NI_MAXHOST];      // Client's remote name
    char service[NI_MAXSERV];   // Service (i.e. port) the client is connect on

    memset(host, 0, NI_MAXHOST); // same as memset(host, 0, NI_MAXHOST);
    memset(service, 0, NI_MAXSERV);
   
  

    // While loop: accept and echo message back to client
    char buf[4096];


  // Try to convert the IP to a human-readable form, otherwise use inet_ntoa
    if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
          cout << "Host connected: " << host << endl;
    } else {
          inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
          cout << "Host connected: " << inet_ntoa(client.sin_addr) << endl;
          RCLCPP_INFO(node->get_logger(), "Publishing: '%s'", inet_ntoa(client.sin_addr));

    }

  while (true)
      {
          memset(buf, 0, 4096);
  
          // Wait for android client to send data
          // Basic msg handling - TODO - Add Complete msg function scheme by (msg size send from client as prefix msg )
          int bytesReceived = recv(clientSocket, buf, 4096, 0);
          if (bytesReceived == -1)
          {
              //cout << "Error in recv(). Quitting" << endl;
              break;
          }
  
          if (bytesReceived == 0)
          {
              //cout << "Client disconnected " << endl;
              break;
          }
          //RCLCPP_INFO(node->get_logger(), buf);
          buf[strcspn(buf, "\n")] = 0;
          message.data = buf;
          //RCLCPP_INFO(node->get_logger(), "Publishing: '%s'", message.data.c_str());
          publisher_->publish(message);
          
          // Echo message back to client
          //send(clientSocket, buf, bytesReceived + 1, 0);
      }
      RCLCPP_INFO(node->get_logger(), "Socket closed");
      close(clientSocket); // Close the client socket and prepare to accept a new connection
 } // while is_running
   // Close listening socket
  close(listening);
  rclcpp::shutdown();
  return 0;
}

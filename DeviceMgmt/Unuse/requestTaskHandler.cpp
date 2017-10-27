#include "requestTaskHandler.h"

#define QCC_MODULE "DMA_APP"

RequestTaskHandler::RequestTaskHandler() 
{

}

void RequestTaskHandler::OnEmptyQueue() 
{

}

void RequestTaskHandler::OnTask(std::unique_ptr<command> command)
{
    command->execute(); 

    QCC_LogMsg(("REquestTaskHandler::OnTask executed"));
//    std::cout<<"\nREquestTaskHandler::OnTask\n" << std::endl;
}

#include "config.h"


ConfigData* CConfig::GetInstance()                                              
{                                                                                
    static ConfigData instance;                                                   
                                                                                 
    return &instance;                                                            
}                                                                                
                                                                                 
CConfig::CConfig()                                                           
{}  
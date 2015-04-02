

#ifndef NETWORK_RELATED_FNS_H_
#define NETWORK_RELATED_FNS_H_

#define AP_SSID_LEN_MAX         (33)
#define ROLE_INVALID            (-5)



long ConfigureSimpleLinkToDefaultState();
void InitializeAppVariables();
long ConnectToNetwork();
int ConfigureMode(int iMode);

void ConnectToNetwork_STA();

void ConnectToNetwork_STA_2();


#endif /* NETWORK_RELATED_FNS_H_ */

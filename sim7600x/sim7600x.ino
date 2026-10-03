unsigned long g_millis=0;

#if 1
int power_pin=2;

char g_sim7600x_sb[512];

#define sim7600xSerial Serial2

/*******************************************************************/
uint8_t sim7600x_sendATcommand(const char* ATcommand, const char* expected_answer, unsigned int timeout)
 {

    uint8_t x=0,  answer=0;
    char response[100];
    unsigned long previous;

    memset(response, '\0', 100);    // Initialize the string
    
    delay(100);
    
    while( sim7600xSerial.available() > 0) sim7600xSerial.read();    // Clean the input buffer
    
    sim7600xSerial.println(ATcommand);    // Send the AT command 
//    sim7600xSerial.print(ATcommand);    // Send the AT command 

    x = 0;
    previous = millis();

    // this loop waits for the answer
    do {
        if(sim7600xSerial.available() != 0){    
            // if there are data in the UART input buffer, reads it and checks for the asnwer
            response[x] = sim7600xSerial.read();      
            Serial.print(response[x]);
            Serial.print(' ');
            Serial.println((int)response[x]);
            x++;
            // check if the desired answer  is in the response of the module
            if (strstr(response, expected_answer) != NULL)    
            {
                answer = 1;
            }
        }
         // Waits for the asnwer with time out
    } while((answer == 0) && ((millis() - previous) < timeout));
    
//    mySerial.print("\n");   

    return answer;
}
/*******************************************************************/
int sim7600x_read(char *msg, size_t msz, size_t *len)
{
  unsigned char c;

  if((*len+1) >= msz) return(-1);

  while(sim7600xSerial.available()) {
    c=(unsigned char)sim7600xSerial.read();
//    Serial.println((char)c);
    if((c != '\r') && (c != '\n')) {
      *(msg+*len)=c;
      (*len)++;
      if((*len+1) >= msz) {
        msg[*len]='\0';
        return(1);
      }
    } else if(c == '\n') {
      msg[*len]='\0';
      return(1);
    }
  }

  return(0);
}
/*******************************************************************/
int sim7600x_read_bin(char *msg, size_t msz, size_t *len)
{
  unsigned char c;

  if(*len >= msz) return(-1);

  while(sim7600xSerial.available()) {
    c=(unsigned char)sim7600xSerial.read();
//    Serial.println((char)c);
    *(msg+*len)=c;
    (*len)++;
    if(*len >= msz) return(1);
  }

  return(0);
}
/*******************************************************************/
int sim7600x_cmd(char *cmd, unsigned long tmo, char *rsp, size_t rsz, int *rc)
{
  int rval=-1,ret;
  size_t len;
  char *p,msg[512];
  unsigned long t0;

  if(rsz <= 1) return(-1);
  t0=millis();
  *rsp='\0';
  *rc=0;

  sim7600xSerial.println(cmd);
  
  len=0;
  for(;;) {
//    Serial.println("reading from sim7600x");
    if((ret=sim7600x_read(msg,sizeof(msg)-1,&len)) == 1) {
      msg[len]='\0';

/*      
      if(len > 0) {
        Serial.print("original msg from sim7600x: ");
        Serial.print(len);
        Serial.print(" ");
        Serial.println(msg);
      }
*/      
      
      if(strcmp(msg,"OK") == 0) {
        *rc=1;
        break;
      }
      if(strcmp(msg,"ERROR") == 0) {
        *rc=2;
        rval=0;
        goto end;
      }
      p=msg;
      while(*p) {
        if(isspace(*p)) p++;
        else break;
      }
      if(*p != '\0') {
        strncpy(rsp,p,rsz);
//        Serial.println(msg);
      }
      len=0;
    } else if(ret != 0) goto end;
    if((millis()-t0) > tmo) {
      msg[len]='\0';
      p=msg;
      while(*p) {
        if(isspace(*p)) p++;
        else break;
      }
      if(*p != '\0') {
        strncpy(rsp,p,rsz);
      }
      *rc=-1;
      rval=0;
      goto end;
    }
    delay(100);
  }

  rval=1;
  
end:

  return(rval);
}
/*******************************************************************/
int sim7600x_prompt(char *cmd, unsigned long tmo)
{
  int rval=-1,ret;
  size_t len;
  char *p,msg[512];
  unsigned long t0;

  t0=millis();

  sim7600xSerial.println(cmd);
  
  len=0;
  for(;;) {
//    Serial.println("reading prompt from sim7600x");
    if((ret=sim7600x_read_bin(msg,1,&len)) == 1) {
      msg[len]='\0';

/*      
      if(len > 0) {
        Serial.print("original prompt msg from sim7600x: ");
        Serial.print(len);
        Serial.print(" ");
        Serial.println(msg);
      }
*/      

      if((len == 1) && (msg[0] == '>')) break;

      p=msg;
      while(*p) {
        if(isspace(*p)) p++;
        else break;
      }
      len=0;
    } else if(ret != 0) goto end;
    if((millis()-t0) > tmo) {
      msg[len]='\0';
      p=msg;
      while(*p) {
        if(isspace(*p)) p++;
        else break;
      }
      rval=0;
      goto end;
    }
    delay(100);
  }

  rval=1;
  
end:

  return(rval);
}
/*******************************************************************/
int sim7600x_cmd_async(char *cmd, unsigned long tmo, char *astr, char *rsp, size_t rsz, int *rc)
{
  int rval=-1,ret,ok=0;
  size_t len;
  char *p,msg[512];
  unsigned long t0;

  if(rsz <= 1) return(-1);
  t0=millis();
  *rsp='\0';
  *rc=0;

  sim7600xSerial.println(cmd);
  
  len=0;
  for(;;) {
//    Serial.println("reading from sim7600x");
    if((ret=sim7600x_read(msg,sizeof(msg)-1,&len)) == 1) {
      msg[len]='\0';

/*      
      if(len > 0) {
        Serial.print("original async msg from sim7600x: ");
        Serial.print(len);
        Serial.print(" ");
        Serial.println(msg);
      }
*/      
      
      if(strcmp(msg,"OK") == 0) {
        *rc=1;
        ok=1;
        len=0;
        continue;
      }
      if(strcmp(msg,"ERROR") == 0) {
        *rc=2;
        ok=0;
        rval=0;
        goto end;
      }
      p=msg;
      while(*p) {
        if(isspace(*p)) p++;
        else break;
      }
      if(*p != '\0') {
        strncpy(rsp,p,rsz);
//        Serial.println(msg);
      }
      if(strstr(msg,astr) != NULL) {
        if(ok == 1) break;
      }
      len=0;
    } else if(ret != 0) goto end;
    if((millis()-t0) > tmo) {
      msg[len]='\0';
      p=msg;
      while(*p) {
        if(isspace(*p)) p++;
        else break;
      }
      if(*p != '\0') {
        strncpy(rsp,p,rsz);
      }
      *rc=-1;
      rval=0;
      goto end;
    }
    delay(100);
  }

  rval=1;
  
end:

  return(rval);
}
/**************************Power on Sim7600x**************************/
int sim7600x_poweron(int pwrpin)
{
  int ret,xx=0,rc=0;
  char *at;
  char msg[512]={0};
  
  Serial.print("Starting up...\n");
    
// power on pulse
  digitalWrite(pwrpin,HIGH);
  delay(500);
  digitalWrite(pwrpin,LOW);
    
// waits for an answer from the module
  for(;;) {     // Send AT every two seconds and wait for the answer
    Serial.print(++xx);
    Serial.println(". AT after power ON");
    at=(char*)"AT";
    if((ret=sim7600x_cmd(at,2000,msg,sizeof(msg),&rc)) == 1) break;
  }


  Serial.println("Waiting ...");
  delay(5000);

/*
Serial.println("Starting AT+CREG?");

  while ((sim7600x_sendATcommand("AT+CREG?", "+CREG: 0,1", 500) || sim7600x_sendATcommand("AT+CREG?", "+CREG: 0,5", 500)) == 0) {
    Serial.println("Waiting AT+CREG?");
    delay(500);
  }
*/

  return(0);
}
/*******************************************************************/
int sim7600x_init(void)
{
  int rval=-1,setpin=0,ret,rc=0;
  int cpinok=0,pinready=0,callready=0,smsready=0;
  int cregok=0,creg1=0,creg5=0;
  char *at=(char*)"";
  char msg[512]={0};
  size_t len;
  char sc;

  Serial.println("sim7600x initialization started ...");

  pinMode(power_pin,OUTPUT);
  digitalWriteFast(power_pin,HIGH);
  delay(500);

  at=(char*)"AT";
  if((ret=sim7600x_cmd(at,2000,msg,sizeof(msg),&rc)) != 1) {
    Serial.print("response read ");
    Serial.print(ret);
    Serial.print(" ");
    Serial.println(msg);
    if(ret == 0) {
      if(strcmp("AT",msg) == 0) {
        Serial.println("Power ON1");
        sim7600x_poweron(power_pin);
      } else {
        Serial.println("Power ON2");
        sim7600x_poweron(power_pin);
//        goto end;
      }
    } else {
      goto end;
    }
  }
//  Serial.println(msg);

  at=(char*)"ATE0";
  if(sim7600x_cmd(at,1000,msg,sizeof(msg),&rc) != 1) goto end;
//  Serial.println(msg);
  at=(char*)"AT+CBC";
  if(sim7600x_cmd(at,1000,msg,sizeof(msg),&rc) != 1) goto end;
  Serial.println(msg);

  at=(char*)"AT+CPIN?";
  if(sim7600x_cmd(at,1000,msg,sizeof(msg),&rc) != 1) goto end;
  Serial.println(msg);
  
  if(strcmp(msg,"+CPIN: SIM PIN") == 0) setpin=1;

  if(setpin == 1) {
    at=(char*)"AT+CPIN=9555";
    sim7600xSerial.println(at);
    len=0;
    for(;;) {
      if((ret=sim7600x_read(msg,sizeof(msg)-1,&len)) == 1) {
        msg[len]='\0';
        if(*msg != '\0') {
          Serial.println(msg);
        }
        if(strcmp(msg,"OK") == 0) cpinok=1;
        else if(strcmp(msg,"+CPIN: READY") == 0) pinready=1;
        else if(strcmp(msg,"PB DONE") == 0) callready=1;
        else if(strcmp(msg,"SMS DONE") == 0) smsready=1;
        if((cpinok == 1) && (pinready == 1) && (callready == 1) && (smsready == 1)) break;
        len=0;
      } else if(ret != 0) goto end;
    }
  }

  at=(char*)"AT+CREG?";
  sim7600xSerial.println(at);
  len=0;
  for(;;) {
    if((ret=sim7600x_read(msg,sizeof(msg)-1,&len)) == 1) {
      msg[len]='\0';
//      if(*msg != '\0') {
//        Serial.println(msg);
//      }
      if(strcmp(msg,"OK") == 0) cregok=1;
      else if(strcmp(msg,"+CREG: 0,1") == 0) creg1=1;
      else if(strcmp(msg,"+CREG: 0,5") == 0) creg5=1;
      if((cregok == 1) && ((creg1 == 1) || (creg5 == 1))) break;
      if(cregok == 1) {
        delay(1000);
        cregok=0;
        sim7600xSerial.println(at);
      }
      len=0;
    } else if(ret != 0) goto end;
    delay(100);
  }
  delay(1000);
  
  at=(char*)"AT+COPS?";
  if((ret=sim7600x_cmd(at,2000,msg,sizeof(msg),&rc)) != 1) goto end;

  at=(char*)"AT+CSQ";
  if(sim7600x_cmd(at,2000,msg,sizeof(msg),&rc) != 1) goto end;
  Serial.println(msg);

  at=(char*)"AT+CGDCONT=1,\"IP\",\"internet.telekom\"";
  if(sim7600x_cmd(at,2000,msg,sizeof(msg),&rc) != 1) goto end;
  Serial.println(msg);

/*
  Serial.println("Closing network");
  at=(char*)"AT+NETCLOSE";
  if(sim7600x_cmd(at,2000,msg,sizeof(msg),&rc) != 1) goto end;
  Serial.println(msg);
  delay(1000);
*/  

  at=(char*)"AT+NETOPEN?";
  if(sim7600x_cmd(at,2000,msg,sizeof(msg),&rc) != 1) goto end;
  Serial.println(msg);
  sc=*(msg+strlen((char*)"+NETOPEN: "));
  if(sc == '1') {
    Serial.println("Network already initialized");
  } else if(sc == '0') {
    Serial.println("Initializing network ...");
    at=(char*)"AT+NETOPEN";
    if(sim7600x_cmd_async(at,2000,(char*)"+NETOPEN: ",msg,sizeof(msg),&rc) != 1) goto end;
    Serial.println(msg);
    sc=*(msg+strlen((char*)"+NETOPEN: "));
    if(sc != '0') {
      Serial.println("Network initialization failed");
      goto end;
    }
    Serial.println("Network successfully initialized");
  } else goto end;

/*
  at=(char*)"AT+NETOPEN?";
  if(sim7600x_cmd(at,2000,msg,sizeof(msg),&rc) != 1) goto end;
  Serial.println(msg);
*/


/*
  at=(char*)"AT+CMGF=1\n";
  if(sim7600x_cmd(at,msg,sizeof(msg),&rc) != 0) goto end;
  Serial.println(msg);
  
  at=(char*)"AT+CMGL=\"ALL\"\n";
  if(sim7600x_cmd(at,msg,sizeof(msg),&rc) != 0) goto end;
  Serial.println(msg);
  
  at=(char*)"AT+CMGD=,4\n";
  if(sim7600x_cmd(at,msg,sizeof(msg),&rc) != 0) goto end;
  Serial.println(msg);
*/  

  at=NULL;

  rval=0;

end:

  if(at == NULL) {
    Serial.println("sim7600x initialization successfully completed.");
  } else {
    Serial.print("sim7600x initialization stucked at: ");
    Serial.println(at);
  }
  
  return(rval);
}
/*******************************************************************/
int nisttstest(void)
{
  int rval=-1,ret,rf=0,ipdset=0,ipclose=0,dataready=0;
  int rc=0;
  char *at;
  char msg[512];
  size_t len,ipd=0,i,j;

  at=(char*)"AT+CIPCLOSE?";
  sim7600x_cmd(at,2000,msg,sizeof(msg),&rc);

  at=(char*)"AT+CIPOPEN=0,\"TCP\",\"129.6.15.28\",13";
  if(sim7600x_cmd_async(at,5000,(char*)"+CIPOPEN: ",msg,sizeof(msg),&rc) != 1) {
    Serial.print(rc);
    Serial.println(" AT+CIPOPEN timed out");
    goto end;
  }
//  Serial.println(msg);

  len=0;
  for(;;) {
    if(sim7600x_read(msg,sizeof(msg),&len) == 1) {

  //Serial.println(msg);

      if(strncmp(msg,(char*)"RECV FROM:",10) == 0) {
//RECV FROM:129.6.15.28:13
        rf=1;
        len=0;
//        Serial.println(msg);
        continue;
      } else if(strncmp(msg,"+IPD",4) == 0) {
//+IPD51
        ipdset=1;
        ipd=atoi(msg+4);
        len=0;
//        Serial.println(msg);
      } else if(strncmp(msg,"+IPCLOSE: ",10) == 0) {
//+IPCLOSE: 0,1
        ipclose=1;
//        Serial.println(msg);
        break;
      }
      if((rf == 1) && (ipdset == 1) && (ipd > 0) && (dataready == 0)) {
//61313 26-09-30 22:25:57 50 0 0 817.2 UTC(NIST) *
//        Serial.println("reading socket data ...");
//        Serial.println(ipd);
        len=0;
        for(;;) {
          if(ipd >= sizeof(msg)) {
            goto end;
          }
          if((ret=sim7600x_read_bin(msg,ipd,&len)) == 1) {
            msg[ipd]='\0';
            dataready=1;
            len=0;
            break;
          } else {
//timeout break     
            delay(100);
          }
        }
//        Serial.println("socket data:");

        for(i=0,j=0;i < ipd;i++) {
          if(iscntrl(msg[i])) continue;
          msg[j++]=msg[i];
        }
        msg[j]='\0';
        
        Serial.println(msg);
      }
    } else {
//      Serial.println("nothing to read");
      delay(100);
    }
  }

  rval=0;

end:

  if(ipclose == 0) {
    at=(char*)"AT+CIPCLOSE?";
    if(sim7600x_cmd(at,2000,msg,sizeof(msg),&rc) == 1) {
      if(rc == 1) {
        if(msg[strlen("+CIPCLOSE: ")] == '0') ipclose=1;
      }
    }
  }

/*
  delay(1000);

  at=(char*)"AT+CIPCLOSE?";
  sim7600x_cmd(at,2000,msg,sizeof(msg),&rc);
  Serial.println(msg);

Serial.println("1-------------------");

  at=(char*)"AT+CIPCLOSE=0";
  sim7600x_cmd(at,2000,msg,sizeof(msg),&rc);
  Serial.println(msg);

Serial.println("2-------------------");
*/

  return(rval);
}
/*******************************************************************/
int tcpbintest(char *txt)
{
  int rval=-1,ret,rf=0,ipdset=0,ipclose=0,dataready=0;
  int rc=0;
  char *at,dat[128];
  char msg[512];
  size_t len,ipd=0,i,j;

  at=(char*)"AT+CIPCLOSE?";
  sim7600x_cmd(at,2000,msg,sizeof(msg),&rc);

  at=(char*)"AT+CIPOPEN=0,\"TCP\",\"45.79.112.203\",4242";
  if(sim7600x_cmd_async(at,5000,(char*)"+CIPOPEN: ",msg,sizeof(msg),&rc) != 1) {
    Serial.print(rc);
    Serial.println(" AT+CIPOPEN timed out");
    goto end;
  }
//  Serial.println(msg);
//delay(250);
  snprintf(dat,sizeof(dat),"AT+CIPSEND=0,%d",strlen(txt)+2);
//  at=(char*)"AT+CIPSEND=0,5";
  at=dat;
//  Serial.println(at);
  if((ret=sim7600x_prompt(at,2000)) != 1) {
    Serial.print(ret);
    Serial.print(" ");
    Serial.println(rc);
    Serial.println("AT+CIPSEND fail");
    goto end;
  }
//  sim7600xSerial.print("xxx\r\n");
  sim7600xSerial.print(txt);
  if(sim7600x_cmd_async((char*)"\r\n",3000,(char*)"+CIPSEND: ",msg,sizeof(msg),&rc) != 1) {
//  if(sim7600x_cmd_async((char*)"xyz\r\n",3000,(char*)"+CIPSEND: ",msg,sizeof(msg)) != 1) {
    Serial.println("+CIPSEND timed out");
    goto end;
  }

  len=0;
  for(;;) {
    if(sim7600x_read(msg,sizeof(msg),&len) == 1) {

//  Serial.println(msg);

      if(strncmp(msg,(char*)"RECV FROM:",10) == 0) {
//RECV FROM:129.6.15.28:13
        rf=1;
        len=0;
//        Serial.println(msg);
        continue;
      } else if(strncmp(msg,"+IPD",4) == 0) {
//+IPD51
        ipdset=1;
        ipd=atoi(msg+4);
        len=0;
//        Serial.println(msg);
//        continue;
      } else if(strncmp(msg,"+IPCLOSE: ",10) == 0) {
//+IPCLOSE: 0,1
        ipclose=1;
//        Serial.println(msg);
        break;
      } else {
        if(len > 0) {
          Serial.println("unexpected data:");
          Serial.println(len);
          Serial.println(msg);
        }
        len=0;
        continue;
      }
      if((rf == 1) && (ipdset == 1) && (ipd > 0) && (dataready == 0)) {
//        Serial.println("reading socket data ...");
//        Serial.println(ipd);
        len=0;
        for(;;) {
          if(ipd >= sizeof(msg)) {
            goto end;
          }
          if((ret=sim7600x_read_bin(msg,ipd,&len)) == 1) {
            msg[ipd]='\0';
            dataready=1;
            len=0;
            break;
          } else {
//timeout break     
            delay(100);
          }
        }
        Serial.print("tcpbin data: ");
        for(i=0,j=0;i < ipd;i++) {
          if(iscntrl(msg[i])) continue;
          msg[j++]=msg[i];
        }
        msg[j]='\0';
        Serial.println(msg);
        break;
      }
    } else {
//      Serial.println(" nothing to read");
      delay(100);
    }
  }

  rval=0;

end:

  if(ipclose == 0) {
    at=(char*)"AT+CIPCLOSE?";
    if(sim7600x_cmd(at,2000,msg,sizeof(msg),&rc) == 1) {
      if(rc == 1) {
        if(msg[strlen("+CIPCLOSE: ")] == '0') ipclose=1;
      }
    }
  }

  if(ipclose == 0) {
    at=(char*)"AT+CIPCLOSE=0";
//    sim7600x_cmd(at,2000,msg,sizeof(msg),&rc);
    sim7600x_cmd_async(at,2000,(char*)"+CIPCLOSE: ",msg,sizeof(msg),&rc);
//    Serial.println(msg);
  }

  return(rval);
}
/*******************************************************************/
void setup()
{
  Serial.begin(115200);
  sim7600xSerial.begin(115200);
  sim7600xSerial.addMemoryForRead(&g_sim7600x_sb,sizeof(g_sim7600x_sb));

  delay(1000);
  
  sim7600x_init();

}
/*******************************************************************/
void loop()
{
  int ret,smsi,rc=0;
  char *at,sms[32];
  static char msg[1024]={0};
  static size_t len=0;
  static unsigned long l_1sectmo=0;
  static unsigned long l_5sectmo=0;
//  static unsigned long l_time=0;
  char txt[128];

  g_millis=millis();
  
//  delay(1000);
  if((g_millis-l_1sectmo) >= 1000) {

    if(l_1sectmo == 0) {
      nisttstest();
      tcpbintest((char*)"amd");
    }


//    Serial.println("loop1");
    l_1sectmo=g_millis;
  }

  if((g_millis-l_5sectmo) >= 5000) {
    nisttstest();
    snprintf(txt,sizeof(txt),"m%ld",g_millis);
    tcpbintest(txt);
    l_5sectmo=g_millis;
  }


/*
//    at=(char*)"AT+CNMI?\n";
    at=(char*)"AT+CMGL=\"ALL\"\n";
    if(sim7600x_cmd(at,msg,sizeof(msg),&rc) != 0) {
      Serial.println("error");
      return;
    }
      Serial.println(msg);
  return;
*/  

/*
  len=0;
  if((ret=sim7600x_read(msg,sizeof(msg)-1,&len)) != 1) {
    msg[len]='\0';
    at=(char*)"AT+CMGL=\"ALL\"\n";
    if(sim7600x_cmd(at,msg,sizeof(msg),&rc) != 0) {
      return;
    }
    if(*msg == '\0') {
//      Serial.println("no sms");
      return;
    } else {
      Serial.print(len);
      Serial.print(" ");
      Serial.println(ret);
      Serial.println(msg);
    }
  }
*/  
 
  if((ret=sim7600x_read(msg,sizeof(msg)-1,&len)) != 1) return;
  len=0;
    
  Serial.println("processing ...");
  Serial.println(msg);

  at=(char*)"+CMTI: \"SM\",";
  if(strncmp(msg,at,strlen(at)) == 0) {
    smsi=atoi(msg+strlen(at));
    sprintf(sms,"AT+CMGR=%d",smsi);
    at=(char*)sms;
//Serial.print("get sms cmd: ");
//Serial.println(at);
    if(sim7600x_cmd(at,2000,msg,sizeof(msg),&rc) == 1) {
      Serial.print("get sms data: ");
      Serial.println(msg);
/*      
      if(strstr(msg,"PTB1") != NULL) {
        servo_pb(1);
      } else if(strstr(msg,"PTB2") != NULL) {
        servo_pb(2);
      }
*/      
//      servo_pb(2);
//      smscmdproc(msg);
    }
    sprintf(sms,"AT+CMGD=%d,0",smsi);
    at=(char*)sms;
Serial.print("del sms cmd: ");
Serial.println(at);
    if(sim7600x_cmd(at,2000,msg,sizeof(msg),&rc) != 1) {
      return;
    }
    Serial.println(msg);
  } else {
    Serial.print("Message from sim7600x: ");
    Serial.println(msg);
  }

}

#endif 


#if 0
// Change "Serial1" to match whichever Serial port you wired your SIM pins to on the Teensy 4
// (e.g., Serial1 uses Pin 0 for RX, Pin 1 for TX)
#define ModemSerial Serial2

const int pwrPin = 2; // Change to your PWR pin number

void setup()
 {
  Serial.begin(115200);
/*  
  while (!Serial && millis() < 3000); // Wait for PC Serial Monitor to open
  
  Serial.println("--- Booting SIM7600X ---");
  
  // Power up the module
  pinMode(pwrPin, OUTPUT);
  digitalWrite(pwrPin, LOW); 
  delay(2500);               
  digitalWrite(pwrPin, HIGH); 
*/  
  // Waveshare default baud rate is usually 115200
  ModemSerial.begin(115200); 
  
  Serial.println("Ready! Type your AT commands in the Serial Monitor.");
}

void loop() {
  // 1. Forward everything from PC Serial Monitor to the SIM Module
  if (Serial.available()) {
    char c = Serial.read();
    ModemSerial.write(c);
  }
  
  // 2. Forward everything from the SIM Module back to the PC Serial Monitor
  if (ModemSerial.available()) {
    char c = ModemSerial.read();
    Serial.write(c);
  }
}
#endif

/*
AT+CGDCONT=1,"IP","internet.telekom"
OK
*/
/*
AT+NETOPEN
OK

+NETOPEN: 0
*/
/*
AT+CIPOPEN=0,"TCP","45.79.112.203",4242
OK

+CIPOPEN: 0,0
*/
/*
AT+CIPSEND=0,5
>12345
OK
*/
/*
AT+CIPOPEN=0,"TCP","129.6.15.28",13
OK

+CIPOPEN: 0,0

RECV FROM:129.6.15.28:13
+IPD51

61313 26-09-30 22:25:57 50 0 0 817.2 UTC(NIST) *

+IPCLOSE: 0,1
*/

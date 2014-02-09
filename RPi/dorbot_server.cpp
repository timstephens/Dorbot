/*
tool to look at MySQL database and return whether a user's key is a valid one or not
The first iteration will just try to connect to the DB
*/

#include <cstdlib>
#include <iostream>
#include <my_global.h>
#include <mysql.h>
#include <stdlib.h>
#include "RF24.h"

using namespace std;

RF24 radio("/dev/spidev0.0", 8000000,25);

void setup(void)
{
    // init radio for reading
    radio.begin();
    radio.enableDynamicPayloads();
    radio.setAutoAck(1);
    radio.setRetries(15,15);
    radio.setDataRate(RF24_1MBPS);
    radio.setPALevel(RF24_PA_MAX);
    radio.setChannel(76);
    radio.setCRCLength(RF24_CRC_16);
    radio.openReadingPipe(1,0xF0F0F0F0E1LL);
	radio.openWritingPipe(0xF0F0F0F0D2LL);
	radio.setPayloadSize(32);
    radio.startListening();
}

void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);        
}

void userRecord(char keyValue[12], char validUser) {
	MYSQL *con = mysql_init(NULL);
	char sqlQuery[64] = "";
	
	sprintf(sqlQuery, "INSERT INTO access (key_id, date, allowed_in) VALUES (\"%s\", NOW(), \"%c\" )", keyValue, validUser);

	if (con == NULL)
	 {   
		fprintf(stderr, "mysql_init() failed\n");
		cout << "Error" << endl;
		exit(1);
       }
       
       if (mysql_real_connect(con, "localhost", "dorbot", "d6z{u8oYH>",
               "door", 0, NULL, 0) == NULL)
       {   
           finish_with_error(con);
       }
       
       if (mysql_query(con, sqlQuery))
       {   
           finish_with_error(con);
       }
       
} //userRecord

//=========================================
char userAllowedIn(char keyValue[12]) {
	MYSQL *con = mysql_init(NULL);
	char sqlQuery[64] = ""; 
	char retval=0;
	//without any sort of error checking, this query could be messed around with to allow an SQL injection attack. Databases could be dropped, false information entered, or the door could be opened without authorisation. §
	sprintf(sqlQuery, "SELECT valid_user FROM users WHERE key_id = \"%s\"", keyValue);

if (con == NULL)
  {
      fprintf(stderr, "mysql_init() failed\n");
      exit(1);
  }  
  
  if (mysql_real_connect(con, "localhost", "dorbot", "d6z{u8oYH>", 
          "door", 0, NULL, 0) == NULL) 
  {
      finish_with_error(con);
  }    
  
  if (mysql_query(con, sqlQuery)) 
  {
      finish_with_error(con);
  }
  
  MYSQL_RES *result = mysql_store_result(con);
  
  if (result == NULL) 
  {
      finish_with_error(con);
  }

  int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;
  
  while ((row = mysql_fetch_row(result))) 
  { 
      for(int i = 0; i < num_fields; i++) 
      { 
	cout << row[i]; 
      } 
	cout << endl;
	retval = row[0][0];
  }

 
	  mysql_free_result(result);
	  mysql_close(con);
	cout << sqlQuery << "Retval " << retval << endl;

	return retval;
} //UserAllowedIn();

void loop(void)
{
    // 32 byte character array is max payload
    char receivePayload[32];
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t ); // Get curret Time.
	char buffer[8];
	char validUser = 0;
	char *splitString;
	char *key_id; 

    while (radio.available())
    {
        // read from radio until payload size is reached
        uint8_t len = radio.getDynamicPayloadSize();
        radio.read(receivePayload, len);

	//Split the incoming data into the command, and the key_id. Then sanitise the key_id to prevent SQLInjection attack.
		
	//Need to query the database to see if the user_id is allowed in
	validUser = userAllowedIn(receivePayload);

        // display payload
	cout << (now->tm_year + 1900) << '-'
         << (now->tm_mon + 1) << '-'
         <<  now->tm_mday << ' '
        << now->tm_hour << ':'
        << now->tm_min << ':'
        <<now->tm_sec << ' '
         << receivePayload << endl;
	if(validUser == '1') {
		cout << "User is valid" << endl;
	} else { 
		cout << "ERROR: User is not valid" << endl;
	}
    }
	if(validUser) {
		radio.stopListening();
		sprintf(buffer, "%c", validUser);

		if(radio.write(buffer, 1)) {
			cout << "Successfully replied" << buffer << endl;
		} else {
			cout << "Error replying" << endl;
		}

		userRecord(receivePayload, validUser);
		validUser = 0;
		radio.startListening();
	}
	usleep(500);
}

int main(int argc, char **argv)
{
	setup(); //Run the radio setup stuff

	printf("MySQL client version: %s\n", mysql_get_client_info());
while(1) loop();

	exit(0);
}

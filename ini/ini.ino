#include "AdvancedOTA.h"
#include "CiniParser.h"
#include "rootPage.h"
#include <FS.h>


ESP8266WiFiMulti WiFiMulti;
ESP8266WebServer server(80);

void setup(){
  Serial.begin(115200);
  wifiSetup(&WiFiMulti, &server);

  server.on("/", handleRoot);
  
  server.on("/ini", handleIniSample);
  server.on("/writeini", handleWriteIni);
  server.on("/setini", handleSetIni);

  //for debug
  server.on("/procini", handleProcIni);
  server.on("/delini", handleDelIni);
  
}

void loop() {
  listener();
}


// http://esp8266.local/
// ↑ここにアクセス
void handleRoot() {
  Serial.println("root");
  server.send(200, "text/html", ROOT_HTML );
}



//- ini File Access Sample ------------------------------------------------------------------------

#define INIFNM "/test.ini"


void handleIniSample(){
  String message = "<HTML><BODY>";

  CiniParser testini;
  if(testini.setIniFileName( INIFNM )){
    message += "File not exist";
    message += testini.createIniFile();
  }

  message += "<CENTER><FONT SIZE=5>";
  message += "ini Test sample page</FONT></CENTER><BR>";


//SPIFFS 情報表示
  message += "<B>SPIFFS information </B><BR>";
  SPIFFS.begin();
  FSInfo fsinfo;
  SPIFFS.info(fsinfo);
  message += "<TABLE border=1>";
  message += "<TR><TD>totalBytes</TD><TD>";
  message += fsinfo.totalBytes;
  message += "</TD></TR>";
  message += "<TR><TD>usedBytes</TD><TD>";
  message += fsinfo.usedBytes;
  message += "</TD></TR>";
  message += "<TR><TD>blockSize</TD><TD>";
  message += fsinfo.blockSize;
  message += "</TD></TR>";
  message += "<TR><TD>pageSize</TD><TD>";
  message += fsinfo.pageSize;
  message += "</TD></TR>";
  message += "<TR><TD>maxOpenFiles</TD><TD>";
  message += fsinfo.maxOpenFiles;
  message += "</TD></TR>";
  message += "<TR><TD>maxPathLength</TD><TD>";
  message += fsinfo.maxPathLength;
  message += "</TD></TR>";
  message += "<TR><TD>use rate</TD><TD>";
  message += (fsinfo.usedBytes / fsinfo.totalBytes * 100.00);
  message += "%";
  message += "</TD></TR>";
  message += "</TABLE><BR><BR>";


//ファイル一覧を表示
  message += "<B>Directry information </B><BR>";
  message += "<TABLE border=1>";
  Dir dir = SPIFFS.openDir("/");
  while ( dir.next() ){
    message += "<TR><TD>";
    message += dir.fileName();
    message += "</TD><TD>";
    File f = dir.openFile("r");
    message += f.size();
    message += "</TD></TR>";
    f.close(); 
  }
  message += "</TABLE><BR><BR>";


//iniファイルの情報表示
  message += "<B>ini information </B><BR>";
  message += "fileName: ";
  message += INIFNM;
  message += "<BR>filesize: ";
  message += testini.getIniFileSize();
  message += " byte<BR><BR>";

  String sect = server.arg("Section");
  String name = server.arg("Name");
  String valu;

  if(sect.length()*name.length()){
    testini.rwIni(sect, name, &valu, READ);
  }

//ini ファイル書き込み
  message += "<B>write ini test </B><BR>";
  message += "<FORM method=POST action=/writeini>";
  message += "Section: <INPUT type=text name='Section' value='";
  message += sect;
  message += "'>Name: <INPUT type=text name='Name' value='";
  message += name;
  message += "'>Value: <INPUT type=text name='Value' value='";
  message += valu;
  message += "'><INPUT type=submit value='Write'></FORM>";


//ini ファイル取得
  message += "<B>read ini test </B><BR>";
  message += "<FORM method=POST action=/ini>";
  message += "Section: <INPUT type=text name='Section' value='";
  message += sect;
  message += "'>Name: <INPUT type=text name='Name' value='";
  message += name;
  message += "'>Value: <INPUT type=text name='Value' value='";
  message += valu;
  message += "'disabled='disabled'><INPUT type=submit value='Read'></FORM>";

  
//ini ファイルの内容を表示
  message += "<B>ini file viewer </B><BR>";
  message += "<FORM method=POST action=/setini><TEXTAREA NAME=ini cols=100 rows=40>";
  testini.readIniFile(&message);
  message += "</TEXTAREA>";
  message += "<INPUT type=submit value='submit'></FORM><BR>";


//WiFi ini 設定
  message += "<B>WiFi ini Setting</B><BR>";
  message += "<FORM method=POST action=/writeini>";
  message += "Section: <INPUT type=text name='Section' value='WiFi' disabled='disabled'>";
  message += "Name: <INPUT type=text name='Name' value='SSID' disabled='disabled'>";
  message += "<INPUT type=hidden name='Section' value='WiFi'>";
  message += "<INPUT type=hidden name='Name' value='SSID' >";
  message += "Value: <INPUT type=text name='Value'>";
  message += "<INPUT type=submit value='Update SSID'></FORM>";
  message += "<FORM method=POST action=/writeini>";
  message += "Section: <INPUT type=text name='Section' value='WiFi' disabled='disabled'>";
  message += "Name: <INPUT type=text name='Name' value='PASS' disabled='disabled'>";
  message += "<INPUT type=hidden name='Section' value='WiFi'>";
  message += "<INPUT type=hidden name='Name' value='PASS' >";
  message += "Value: <INPUT type=text name='Value'>";
  message += "<INPUT type=submit value='Update PASS'></FORM>";
  
  message += "</BODY></HTML>";
  server.send(200, "text/html", message);
}


void handleWriteIni(){
  CiniParser testini;
  if(testini.setIniFileName( INIFNM )){
    Serial.println( "File not exist");
    handleIniSample();
    return;
  }

  String sect = server.arg("Section");
  String name = server.arg("Name");
  String valu = server.arg("Value");

  if(sect.length()*name.length()){
    testini.rwIni(sect, name, &valu, WRITE);
  }
  
  server.sendHeader("Location", "/ini", true);
  server.send(301, "text/plain", "");
}

void handleSetIni(){
  String ini = server.arg("ini");
  
  CiniParser testini;
  if(testini.setIniFileName( INIFNM )){
    Serial.println("File not exist");
  }else{
    testini.writeIniFile(&ini);
  }

  server.sendHeader("Location", "/ini", true);
  server.send(301, "text/plain", "");
}


void handleProcIni(){
  String message = "<HTML><BODY>";

  SPIFFS.begin();
  
  FSInfo fsinfo;
  SPIFFS.info(fsinfo);

  message += "<TABLE border=1>";
  message += "<TR><TD>totalBytes</TD><TD>";
  message += fsinfo.totalBytes;
  message += "</TD></TR>";
  message += "<TR><TD>usedBytes</TD><TD>";
  message += fsinfo.usedBytes;
  message += "</TD></TR>";
  message += "<TR><TD>blockSize</TD><TD>";
  message += fsinfo.blockSize;
  message += "</TD></TR>";
  message += "<TR><TD>pageSize</TD><TD>";
  message += fsinfo.pageSize;
  message += "</TD></TR>";
  message += "<TR><TD>maxOpenFiles</TD><TD>";
  message += fsinfo.maxOpenFiles;
  message += "</TD></TR>";
  message += "<TR><TD>maxPathLength</TD><TD>";
  message += fsinfo.maxPathLength;
  message += "</TD></TR>";
  message += "<TR><TD>use rate</TD><TD>";
  message += (fsinfo.usedBytes / fsinfo.totalBytes * 100.00);
  message += "%";
  message += "</TD></TR>";
  message += "</TABLE><BR><BR>";

  message += "<TABLE border=1>";
  Dir dir = SPIFFS.openDir("/");
  while ( dir.next() ){
    message += "<TR><TD>";
    message += dir.fileName();
    message += "</TD><TD>";
    File f = dir.openFile("r");
    message += f.size();
    message += "</TD></TR>";
    message += "<TR><TD>";
            CiniParser testini;
            if(testini.setIniFileName( (char*)f.name() )){
              message += "didn't open";
            }
            testini.readIniFile(&message);
    message += "</TD></TR>";
    f.close();
 
  }
  message += "</TABLE><BR><BR>";
  

  message += "<FORM method=POST action=/setini><TEXTAREA NAME=ini cols=100 rows=40>";

            CiniParser testini;
            if(testini.setIniFileName( "/temp.ini" )){
              message += "didn't open";
            }
            testini.readIniFile(&message);

  message += "</TEXTAREA>";
  message += "<INPUT type=submit value='submit'></FORM>";

  message += "</BODY></HTML><FORM>";

  SPIFFS.end();
  server.send(200, "text/html", message);

}

void handleDelIni(){
  String message = "<HTML><BODY>";

  CiniParser testini;
  if(testini.setIniFileName( INIFNM )){
    message += "File not exist";
  }
  message += testini.deleteIniFile();

  message += "<FORM method=POST action=/setini><TEXTAREA NAME=ini cols=100 rows=40>";
  
  testini.readIniFile(&message);

  message += "</TEXTAREA>";
  message += "<INPUT type=submit value='submit'></FORM>";
  message += "</BODY></HTML><FORM>";
  server.send(200, "text/html", message);
}



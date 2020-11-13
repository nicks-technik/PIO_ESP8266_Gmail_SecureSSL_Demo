// You can check the file alternative for an other way

/*
 * This demo sketch will fail at the Gmail login unless your Google account has
 * set the following option:
 *
 *     Allow less secure apps: OFF
 *
 * which can be found in your Google account under Security. You will get an email
 * warning from Google that the universe will implode if you allow this.
 *
 * see https://www.base64encode.org/ for encoding / decoding
 */

#include <Arduino.h>

#include <base64.h>
#include <ESP8266WiFi.h>

// Contains secured Password and other Setup Variables
#include "secure.h"
/*
 * const char* _WIFI_SSID = "Greypuss";
 * const char* _WIFI_Password = "mypassword";
 * const char* _GMailServer = "smtp.gmail.com";
 * const char* _mailUser = "mygmailaddress@gmail.com";
 * const char* _mailPassword = "my Google password";
 * const String _To = "To: Home Alone Group<totally@made.up>";
 * const String _From = "From: expressifmail+test@gmail.com";
*/

const char *_VAR_Fingerprint = "289509731da223e5218031c38108dc5d014e829b"; // For smtp.gmail.com

WiFiClientSecure _WifiClientSecure;

// Forward declarations of functions (only required in Eclipse IDE)
byte response();
byte sendEmail();

void setup()
{
  delay(500);
  Serial.begin(115200);

  // Connect to WiFi network
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(_WIFI_SSID);

  WiFi.begin(_WIFI_SSID, _WIFI_Password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("My IP address: ");
  Serial.println(WiFi.localIP());

  delay(1000);
  sendEmail();
}

void loop()
{
  //Nothing to do here. Never send email in a loop. You will get blacklisted.
}

// Function send a secure email via Gmail
byte sendEmail()
{
  _WifiClientSecure.setFingerprint(_VAR_Fingerprint); // not available in axTLS::WiFiClientSecure 4.2.2
  // if (!response())
  //   return 0;
  // port 465=SSL 567=TLS; 587 not available with library 4.2.2
  // this all needs Google security downgrading:
  // https://myaccount.google.com/lesssecureapps?utm_source=google-account&utm_medium=web

  /*
 * Gmail exposes port 465 for SMTP over SSL and port 587 for SMTP with STARTTLS.
 * The difference between these two is that SMTP over SSL first establishes a secure 
 * SSL/TLS connection and conducts SMTP over that connection, and SMTP with STARTTLS 
 * starts with unencrypted SMTP and then switches to SSL/TLS. 
 * See https://stackoverflow.com/questions/17281669/using-smtp-gmail-and-starttls
 */
  Serial.println("Attempting to connect to GMAIL server");
  _WifiClientSecure.setInsecure();

  if (_WifiClientSecure.connect("smtp.gmail.com", 465) == 1)
  {
    Serial.println(F("Connected"));
  }
  else
  {
    Serial.print(F("Connection failed:"));
    return 0;
  }
  if (!response())
    return 0;

  Serial.println(F("Sending Extended Hello"));
  _WifiClientSecure.println("EHLO gmail.com");
  if (!response())
    return 0;

  // We're not using port 567 in this demo
  //Serial.println(F("STARTTLS"));
  //if (!response())
  //  return 0;
  //Serial.println(F("Sending EHLO"));
  //_WifiClientSecure.println("EHLO gmail.com");
  //if (!response())
  //  return 0;

  Serial.println(F("Sending auth login"));
  _WifiClientSecure.println("auth login");
  if (!response())
    return 0;

  Serial.println(F("Sending User"));
  // Change to your base64, ASCII encoded user
  _WifiClientSecure.println(base64::encode(_mailUser));
  if (!response())
    return 0;

  Serial.println(F("Sending Password"));
  // change to your base64, ASCII encoded password
  _WifiClientSecure.println(base64::encode(_mailPassword));
  if (!response())
    return 0;

  Serial.println(F("Sending From"));
  // your email address (sender) - MUST include angle brackets
  // _WifiClientSecure.println(F("MAIL FROM: <arduinitepower@gmail.com>"));
  _WifiClientSecure.println("MAIL " + _From);
  if (!response())
    return 0;

  // change to recipient address - MUST include angle brackets
  Serial.println(F("Sending To"));
  // _WifiClientSecure.println(F("RCPT To: <ralph@gmail.com>"));
  _WifiClientSecure.println("RCPT " + _To);
  // Repeat above line for EACH recipient
  if (!response())
    return 0;

  Serial.println(F("Sending DATA"));
  _WifiClientSecure.println(F("DATA"));
  if (!response())
    return 0;

  Serial.println(F("Sending email"));
  // recipient address (include option display name if you want)
  // _WifiClientSecure.println(F("To: Home Alone Group<totally@made.up>"));
  _WifiClientSecure.println(_To);

  // change to your address
  // _WifiClientSecure.println(F("From: test@gmail.com"));
  // _WifiClientSecure.println(_From);
  _WifiClientSecure.println(F("Subject: Your Arduino\r\n"));
  _WifiClientSecure.println(F("This email was sent securely via an encrypted mail link.\n"));
  _WifiClientSecure.println(F("In the last hour there was: 8 activities detected. Please check all is well."));
  _WifiClientSecure.println(F("This email will NOT be repeated for this hour.\n"));
  _WifiClientSecure.println(F("This email was sent from an unmonitored email account - please do not reply."));
  _WifiClientSecure.println(F("Love and kisses from Dougle and Benny. They wrote this sketch."));

  // IMPORTANT you must send a complete line containing just a "." to end the conversation
  // So the PREVIOUS line to this one must be a prinln not just a print
  _WifiClientSecure.println(F("."));
  if (!response())
    return 0;

  Serial.println(F("Sending QUIT"));
  _WifiClientSecure.println(F("QUIT"));
  if (!response())
    return 0;

  _WifiClientSecure.stop();
  Serial.println(F("Disconnected Client"));
  return 1;
}

// Check response from SMTP server
byte response()
{
  // Wait for a response for up to X seconds
  int loopCount = 0;
  while (!_WifiClientSecure.available())
  {
    delay(1);
    loopCount++;
    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000)
    {
      _WifiClientSecure.stop();
      Serial.println(F("\r\nTimeout"));
      return 0;
    }
  }

  // Take a snapshot of the response code
  byte respCode = _WifiClientSecure.peek();
  while (_WifiClientSecure.available())
  {
    Serial.write(_WifiClientSecure.read());
  }

  if (respCode >= '4')
  {
    Serial.print("Failed in eRcv with response: ");
    Serial.print(respCode);
    return 0;
  }
  return 1;
}


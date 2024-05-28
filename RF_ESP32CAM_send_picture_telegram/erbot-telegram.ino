///////////////////////////////////////////////////////////////////////////////////////////
void telegram_sendImage(camera_fb_t* inImage) {
  digitalWrite(erFlashLed, 1);
  delay(1000);
  digitalWrite(erFlashLed, 0);
  sendPhoto = true;
  Serial.println("gambar dalam proses");
  sendPhotoTelegram(inImage);
  sendPhoto = false;
  Serial.println("Done");
}
///////////////////////////////////////////////////////////////////////////////////////////
void telegram_readMessage() {
  if (millis() > lastTimeBotRan + botRequestDelay) {
    lastTimeBotRan = millis();
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
  }
}
///////////////////////////////////////////////////////////////////////////////////////////
/**/
void telegram_messageStart() {
  String msg;
  msg += " --> STATUS  \n\n";
  msg += "Camera : " + support_convert1toTrue(function_checkCamera());
  msg += "Memory : " + support_convert1toTrue(function_checkSdcard());
  msg += "\n";
  msg += "ketik '/info' untuk melihat fungsi lain dari alat...!\n";
  bot.sendMessage(CHAT_ID, msg, "");
}
///////////////////////////////////////////////////////////////////////////////////////////
void telegram_messageInfo() {
  String msg1;
  msg1 += "--> INFO \n\n";
  msg1 += "commands are responded to : \n\n";
  msg1 += "/info : tool response info \n";
  msg1 += "/status : device status \n";
  msg1 += "/flash : turn on/off flash led \n";
  msg1 += "/image : send a recent picture \n";
  msg1 += "\n";
  bot.sendMessage(CHAT_ID, msg1, "");
}
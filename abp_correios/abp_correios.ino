#include <SPI.h>
#include <MFRC522.h> 

#define sda_pin   10
#define reset_pin  9
#define error      8
#define success    7
#define buzz       5
#define branch_A   4
#define branch_B   3
#define branch_C   2

const byte output_pins[] = {branch_A, branch_B, branch_C, error, success, buzz};

typedef struct
{
  String id;
  char   branch;
} track_type;

track_type tag[100];

byte index = 0;
String strID = "";
char branch = 'A';

MFRC522 rfid(sda_pin, reset_pin);

void read_tag(){
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) 
    return;
    
  for (byte i = 0; i < 4; i++) {
    strID +=
    (rfid.uid.uidByte[i] < 0x10 ? "0" : "") +
    String(rfid.uid.uidByte[i], HEX) +
    (i!=3 ? ":" : "");
  }
  rfid.PICC_HaltA(); //Input finished
  rfid.PCD_StopCrypto1(); //Cryptography finished

  strID.toUpperCase();
  handle_new_entry(strID, branch);
  strID = "";
}

void handle_new_entry(String id, char branch){
  if(!verify_entry_existance(id)){      
    tag[index].id = id;
    tag[index].branch = branch;
    index++;
    Serial.println("Tag with ID " + id + " entered in branch " + branch + "!");
    digitalWrite(success, HIGH);
    tone(buzz, 5000);
    delay(500);
    noTone(buzz);
    digitalWrite(success,  LOW);
  }
  else{
    for(int i = 0; i <= index; i++)
    {
      if(tag[i].id == strID){
        if(tag[i].branch == branch){
          Serial.print("Tag with ID ");
          Serial.print(tag[i].id);
          Serial.print(" is lefting branch ");
          Serial.print(tag[i].branch);
          Serial.println("!");
          digitalWrite(success, HIGH);
          tone(buzz, 5000);
          delay(500);
          noTone(buzz);
          digitalWrite(success,  LOW);
          
          tag[i].id     = "0";
          tag[i].branch = '0';
        }
        else{
          Serial.println("DENIED -> Tag " + tag[i].id + " is still in the branch " + String(tag[i].branch) + "!");
          digitalWrite(error, HIGH);
          tone(buzz, 5000);
          delay(2000);
          noTone(buzz);
          digitalWrite(error,  LOW);
        }
      }
    }
  }
}

bool verify_entry_existance(String id){
  bool stop = 0;
  for(int i = 0; i <= index; i++){
    if(tag[i].id == id){
      stop = 1;
    }
  } 
  return stop;
}

void write_tag_list(){
  Serial.println("");
  Serial.println("Tag          Branch");
  for(int i = 0; i <= index; i++)
  {
    if((tag[i].id != "0")&&(tag[i].id != "")&&(tag[i].branch != '0')&&(tag[i].branch != ' ')){
      Serial.print(tag[i].id);
      Serial.print(" |   ");
      Serial.println(tag[i].branch);
    }
  }
}

void set_branch(char p_branch)
{
  branch = p_branch;
  String str_msg = "Branch set to ";
  Serial.println(str_msg + branch);

  for(int i = 0; i <= 2; i++)
  {
    digitalWrite(output_pins[i], LOW);
  }
  if (branch == 'A')
    digitalWrite(branch_A, HIGH);
  else if (branch == 'B')
    digitalWrite(branch_B, HIGH);
  else if (branch == 'C')
    digitalWrite(branch_C, HIGH);
}

void startup(){
  for(int i = 0; i <= sizeof(output_pins); i++)
  {
    pinMode(output_pins[i], OUTPUT);
  }
  digitalWrite(branch_A, HIGH);
  digitalWrite(branch_B,  LOW);
  digitalWrite(branch_C,  LOW);
  digitalWrite(error,     LOW);
  digitalWrite(success,   LOW);
  noTone(buzz);
}

void setup(){
  startup();
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
}

void loop(){
  read_tag();
  
  if(Serial.available() == 0) {}    
  String serial_str = Serial.readString();
  serial_str.trim(); 
  
  if (serial_str == "listar") {
    write_tag_list();
  }
  else if (serial_str == "branch A") {
    set_branch('A');
  }
  else if (serial_str == "branch B") {
    set_branch('B');
  }
  else if (serial_str == "branch C") {
    set_branch('C');
  }
}

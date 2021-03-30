
// static config for faster startup (takes 6s with DHCP)
// 4th byte of the IP address 192.168.1.X
// hardcoded to work in zelial's home password. Have different layout?
// edit the utils.cpp library.
const byte ip_last_byte = 15;

// WiFi credentials
const char* ssid = "wifi_name";
const char* wifi_password = "wifi_password";

//code version
const String version = "1.0";

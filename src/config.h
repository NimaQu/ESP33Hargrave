const int fanCsPinCWF = 9;
const int tempCsPinCWF = 10;

// float temp = 24.0; // Temperature in degrees C
char ssid[] = "Fubuki Networks"; // your network SSID (name)
char pass[] = "1145141919"; // your network password
const String apiKeys[] = {"1145141919"};

enum FanMode
{
  High,
  Med,
  Low,
  Auto,
  Off,
  FanModeCount
};

const int FanResistances[FanModeCount] = {16130, 13320, 10770, 2320, 4870};
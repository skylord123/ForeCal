// Clay Configuration definition

var APP_VER = "v4.0";
module.exports = [
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Appearance/Misc."
      },
      {
        "type": "select",
        "appKey": "ColorScheme",
        "defaultValue": "Auto",
        "label": "Color Scheme",
        "description": "(Auto = Black on White during day, White on Black at night)",
        "options": [
          { 
            "label": "Auto", 
            "value": "Auto" 
          },
          { 
            "label": "White on Black",
            "value": "WhiteOnBlack" 
          },
          { 
            "label": "Black on White",
            "value": "BlackOnWhite" 
          }
        ],
        "attributes": {
          "required": "required"
        }
      },
      {
        "type": "toggle",
        "appKey": "ShowBT",
        "label": "Show Bluetooth Status",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "appKey": "BTVibes",
        "label": "Vibrate on Connect/Disconnect",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "appKey": "ShowBatt",
        "label": "Show Battery Status",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "appKey": "ShowWind",
        "capabilities": ["NOT_ROUND"],
        "label": "Show wind speed instead of day name",
        "defaultValue": false
      },
      {
        "type": "select",
        "appKey": "DateFormat",
        "defaultValue": "0",
        "label": "Date Format",
        "options": [
          { 
            "label": "Month, Day", 
            "value": "0" 
          },
          { 
            "label": "Day, Month",
            "value": "1" 
          }
        ],
        "attributes": {
          "required": "required"
        }
      },
      {
        "type": "toggle",
        "appKey": "ShowWeek",
        "label": "Show Week Number",
        "description": "Displays instead of the AM/PM indicator when 12 hour clock is selected.",
        "defaultValue": false
      },
      {
        "type": "toggle",
        "appKey": "ShowSteps",
        "capabilities": ["HEALTH"],
        "label": "Show Steps Progress",
        "description": "Displays as a progress bar for average steps behind the Today/Tomorrow bar.",
        "defaultVaue": false
      }
    ]
  },
  
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Weather Settings"
      },
      {
        "type": "input",
        "appKey": "WeatherLoc",
        "defaultValue": "",
        "label": "Fixed Location",
        "description": "Leave blank to use GPS",
        "attributes": {
          "placeholder": "eg: New York, NY",
          "limit": 100,
        }
      },
      {
        "type": "radiogroup",
        "appKey": "WeatherProvider",
        "label": "Weather Provider",
        "defaultValue": "0",
        "options": [
          {
            "label": "YAHOO!",
            "value": "0"
          },
          {
            "label": "OpenWeatherMap.org",
            "value": "1"
          }
        ]
      },
      {
        "type": "toggle",
        "appKey": "PreferNWS",
        "label": "Prefer NWS (weather.gov) for USA locations",
        "description": "When enabled, will attempt to use NWS first. Automatically falls back to the selected provider above if the location is outside USA coverage.",
        "defaultValue": false
      },
      {
        "type": "input",
        "appKey": "ForecastTime",
        "defaultValue": "18:00",
        "label": "Show tomorrow's forecast after",
        "attributes": {
          "type": "time",
          "required": "required"
        }
      },
      {
        "type": "select",
        "appKey": "TempUnit",
        "defaultValue": "Auto",
        "label": "Temp/Wind Speed Units",
        "options": [
          { 
            "label": "Auto", 
            "value": "Auto" 
          },
          { 
            "label": "Celcius, m/s",
            "value": "C" 
          },
          { 
            "label": "Fahrenheit, mph",
            "value": "F" 
          }
        ],
        "attributes": {
          "required": "required"
        }
      }
    ]
  },
  
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Calendar"
      },
      {
        "type": "select",
        "appKey": "FirstDay",
        "defaultValue": "0",
        "label": "First Day of Week",
        "options": [
          { 
            "label": "Sunday", 
            "value": "0" 
          },
          { 
            "label": "Monday",
            "value": "1" 
          }
        ],
        "attributes": {
          "required": "required"
        }
      },
      {
        "type": "select",
        "appKey": "CalOffset",
        "defaultValue": "0",
        "label": "Calendar Weeks",
        "options": [
          { 
            "label": "Last wk + this wk + next wk", 
            "value": "0" 
          },
          { 
            "label": "This week + next 2 weeks",
            "value": "7" 
          }
        ],
        "attributes": {
          "required": "required"
        }
      }
    ]
  },
  
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Quiet Time"
      },
      {
        "type": "input",
        "appKey": "QTStart",
        "defaultValue": "00:15",
        "label": "Start",
        "attributes": {
          "type": "time",
          "min": "00:00",
          "max": "23:59",
          "required": "required"
        }
      },
      {
        "type": "input",
        "appKey": "QTEnd",
        "defaultValue": "06:30",
        "label": "End",
        "attributes": {
          "type": "time",
          "min": "00:00",
          "max": "23:59",
          "required": "required"
        }
      },
      {
        "type": "toggle",
        "appKey": "QTVibes",
        "label": "Vibrate on BT Disconnect/Connect during Quiet Time",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "appKey": "QTFetch",
        "label": "Fetch weather during Quiet Time",
        "defaultValue": false
      }
    ]
  },

  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Battery Webhook Settings"
      },
      {
        "type": "toggle",
        "appKey": "BatteryWebhookEnabled",
        "label": "Enable Battery Webhook",
        "description": "Send battery data to a webhook endpoint when settings are saved",
        "defaultValue": false
      },
      {
        "type": "input",
        "appKey": "BatteryWebhookURL",
        "defaultValue": "",
        "label": "Webhook URL",
        "description": "e.g., https://example.com/api/battery",
        "attributes": {
          "placeholder": "https://example.com/api/battery",
          "limit": 500
        }
      },
      {
        "type": "select",
        "appKey": "BatteryWebhookMethod",
        "defaultValue": "POST",
        "label": "HTTP Method",
        "options": [
          {
            "label": "POST",
            "value": "POST"
          },
          {
            "label": "PUT",
            "value": "PUT"
          },
          {
            "label": "GET",
            "value": "GET"
          }
        ]
      },
      {
        "type": "toggle",
        "appKey": "BatteryWebhookUseAuth",
        "label": "Use Bearer Authentication",
        "description": "Enable to include a Bearer token in the Authorization header",
        "defaultValue": false
      },
      {
        "type": "input",
        "appKey": "BatteryWebhookAuthToken",
        "defaultValue": "",
        "label": "Bearer Token",
        "description": "Your authentication token (only used if Bearer Authentication is enabled)",
        "attributes": {
          "placeholder": "your-bearer-token-here",
          "limit": 500,
          "type": "password"
        }
      }
    ]
  },
  
  {
    "type": "submit",
    "defaultValue": "Save"
  },
  
  {
    "type": "text",
    "defaultValue": APP_VER
  }
];
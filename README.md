ForeCal
=======

Pebble watchface with weather, forecast, and a compact 3-week calendar.

Support
-------

If ForeCal has been useful to you, consider supporting its development.
All of my projects are fully open-source and free — donations help cover time, tools, and ongoing maintenance.

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/B0B51BM7C)
[![Donate with PayPal](https://www.paypalobjects.com/en_US/i/btn/btn_donate_SM.gif)](https://www.paypal.com/donate/?hosted_button_id=4VS2UQWDUALXA)


Features
--------

- Current time, date, optional week number, and AM/PM display.
- Current temperature, conditions, and icons.
- Today/Tomorrow forecast with high/low temperature and condition text.
- Sunrise/sunset time with automatic day/night color scheme.
- Optional wind speed display (rectangular watches only).
- Bluetooth and battery status icons with optional vibrate alerts.
- Quiet time window (controls Bluetooth vibes and weather fetches).
- 3-week calendar with configurable first day of week and week offset.
- Steps progress bar on health-capable color Pebbles.
- Weather providers: OpenWeatherMap, Open-Meteo, and optional NWS fallback for US locations.
- GPS or fixed-location weather support and configurable update interval.

Remote sync
-----------

ForeCal can send battery status updates to a custom HTTP endpoint (POST with optional Bearer token).

Remote timeline sync
--------------------

The watchface polls a remote endpoint for timeline pins and inserts them locally using the Pebble timeline APIs. I added this because the Core Devices Pebble mobile app currently only supports apps running on the phone adding timeline pins. Remote Timeline Sync restores remote timeline support so you can sync pins from your own server, and it fits perfectly in a watch face since it’s what you see 99% of the time. I would have built this as a background app, but Pebble background apps do not have internet access.

Read the full setup and flow details in [docs/timeline-sync/README.md](docs/timeline-sync/README.md) if you want to use Node-RED for this.

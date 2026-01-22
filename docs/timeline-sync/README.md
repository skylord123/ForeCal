# Remote Timeline Sync

ForeCal supports syncing timeline pins from a remote endpoint. You can build your own endpoint to serve pins, or use the provided Node-RED setup.

This integration uses the local Pebble Timeline emulation built into the `@skylord123/node-red-pebble-timeline` nodes. Pins are stored locally on the Node-RED host, so no Rebble Web Services or timeline tokens are required.

I added this to the watch face because the Core Devices Pebble mobile app currently only supports apps running on the phone adding timeline pins. Remote Timeline Sync brings back remote timeline support for the new Core Devices app, so you can sync pins from your own server. It also fits perfectly in a watch face since it’s what you see on the watch 99% of the time. I would have built this as a background app instead, but Pebble background apps do not have internet access, so they can’t fetch remote pins. That’s why the watch face is the right place for it.

## Prerequisites

Before setting up remote timeline sync, you need:

1. **Node-RED** installed and running
2. **`@skylord123/node-red-pebble-timeline`** module installed
3. **Node-RED flow configured** (see [NODE-RED-SETUP.md](NODE-RED-SETUP.md))

## Configuration

In the ForeCal settings on your phone, configure the **Remote Timeline Sync** section:

| Setting | Description |
|---------|-------------|
| **Endpoint URL** | The URL to fetch timeline pins from (GET request) |
| **Sync Interval** | How often to check for updates (in minutes, default: 15) |
| **Authorization Token** | Bearer token for authentication (must match your Node-RED endpoint) |

## How It Works

1. ForeCal polls your Node-RED endpoint at the configured interval
2. Node-RED reads timeline pins from the local Pebble Timeline store
3. The endpoint returns a JSON array of timeline pins
4. ForeCal compares each pin's content hash with the locally stored hash
5. Only changed or new pins are updated (via delete + insert)
6. Pins that no longer appear in the response are removed from the timeline

This approach is efficient - pins are only modified when their content actually changes.

## Expected Response Format

Your endpoint should return JSON in this format:

```json
{
  "pins": [
    {
      "id": "unique-pin-id",
      "time": "2026-01-16T14:00:00Z",
      "layout": {
        "type": "genericPin",
        "title": "Event Title",
        "subtitle": "Optional subtitle",
        "body": "Optional longer description"
      }
    }
  ]
}
```

### Pin Structure

| Field | Required | Description |
|-------|----------|-------------|
| `id` | Yes | Unique identifier for the pin (used for updates/deletes) |
| `time` | Yes | ISO 8601 timestamp for when the pin should appear |
| `layout` | Yes | Pin layout object (see below) |
| `actions` | No | Array of actions (buttons) for the pin |

### Layout Types

The `layout.type` field determines the pin's appearance:

- `genericPin` - Standard pin with title, subtitle, and body
- `calendarPin` - Calendar-style event
- `reminderPin` - Reminder notification
- `weatherPin` - Weather information display

### Layout Fields

| Field | Description |
|-------|-------------|
| `type` | Layout type (see above) |
| `title` | Main title text (required) |
| `subtitle` | Secondary text line |
| `body` | Longer description text |
| `tinyIcon` | Icon resource for the pin |

## Node-RED Setup

The included Node-RED flow provides a complete solution for serving timeline pins to ForeCal using the local timeline store.

**[See the full Node-RED Setup Guide](NODE-RED-SETUP.md)** for detailed instructions.

### Quick Start (Node-RED)

1. Install the `@skylord123/node-red-pebble-timeline` module in Node-RED
2. Import `flows/timeline-endpoint.json` into Node-RED
3. Configure the Pebble Timeline nodes with a Local Timeline config (empty API URL)
4. Update the Auth Check node with your desired authorization token
5. Configure ForeCal with your Node-RED endpoint URL (e.g., `http://your-server:1880/api/timeline`)

### Example: Home Assistant Calendar -> Timeline

If you want to sync a Home Assistant calendar into your Pebble timeline, use the example flow in `flows/sync-home-assistant-calendar-to-timeline.json`.

Read the setup notes in [flows/sync-home-assistant-calendar-to-timeline.md](flows/sync-home-assistant-calendar-to-timeline.md). You must:
- Update the **calendar.get_events** node to point at your calendar entity
- Customize the **Update Timeline** function node to control how pins are created (layout type, reminders, icon, title/body, etc.)

### Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        Node-RED                              │
│                                                              │
│  ┌──────────────┐    ┌─────────────────┐                    │
│  │ Your Flows   │───>│ Pebble Timeline │                    │
│  │ (HA, MQTT,   │    │   Insert Node   │                    │
│  │  webhooks)   │    └────────┬────────┘                    │
│  └──────────────┘             │                              │
│                               v                              │
│                    ┌──────────────────┐                     │
│                    │ Local Timeline   │                     │
│                    │ Store (filesystem│                     │
│                    │  backed)         │                     │
│                    └────────┬─────────┘                     │
│                             │                                │
│                             v                                │
│  ┌──────────────────────────────────────┐                   │
│  │     GET /api/timeline endpoint       │<──── ForeCal      │
│  │  (reads local store, filters by date)│      polls        │
│  └──────────────────────────────────────┘                   │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### Date Filtering

The endpoint automatically:
- Returns pins from 24 hours ago to 7 days in the future
- Removes pins older than 24 hours from the local cache
- This keeps the timeline relevant and prevents stale data

## Security

### Authentication

If you set an Authorization Token in ForeCal settings, the request will include:

```
Authorization: Bearer your-token-here
```

The Node-RED endpoint flow includes token validation. By default, the token is set to `example-token` - **you should change this** to a secure value.

### Network Security

- Use HTTPS in production environments
- Consider using a reverse proxy (nginx, Caddy) for SSL termination
- Restrict access to your Node-RED instance appropriately

## Troubleshooting

### Pins not appearing

1. Ensure all Pebble Timeline nodes use the same Local Timeline config
2. Verify the config node has an empty API URL
3. Check that your endpoint URL is correct and accessible
4. Verify the JSON response format matches the expected structure
5. Ensure the Node-RED process can write to its local storage
6. Ensure pin times are in ISO 8601 format with timezone
7. Check Node-RED debug output for errors

### Pins not updating

- ForeCal uses content hashing - only pins with changed content will be updated
- The hash includes: `time`, `layout`, and `actions`
- Changing only the `id` will not trigger an update

### Debug Mode

Enable debug logging in ForeCal by checking the console output in the Pebble developer connection. Look for `[App]` prefixed messages related to timeline sync.

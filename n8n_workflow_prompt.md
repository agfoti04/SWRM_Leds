# n8n Workflow: Google Drive Image to LED Frame Array

## Workflow Overview

Build an n8n automation that:
1. Scans a Google Drive folder for new images
2. Converts images to 64x64 RGB hex arrays in C++ format
3. Exposes an REST API endpoint for ESP32 devices to fetch the latest frame every 30 seconds

---

## Workflow Name
**"GoogleDrive_ImageToLED_FrameServer"**

---

## Components & Nodes

### **Node 1: Google Drive Trigger (Poll)**
**Type:** Google Drive Trigger (Polling)

**Configuration:**
- **Action:** Scan folder for new/updated files
- **Google Drive Folder:** `LED_Images` (replace with actual folder name or ID)
- **File Type Filter:** Images (PNG, JPG, JPEG, BMP)
- **Poll Interval:** Every 2 minutes (or adjust as needed)
- **Output:** File metadata (file_id, name, mimetype, modifiedTime)

**Purpose:** Detect when new images are uploaded to the Google Drive folder

---

### **Node 2: File Exists Check**
**Type:** IF Statement

**Condition:**
```
if (file is image AND file_id != last_processed_file_id)
```

**Purpose:** Avoid re-processing the same image

---

### **Node 3: Download Image from Google Drive**
**Type:** Google Drive > Download File

**Configuration:**
- **File ID:** `{{ $node["Google Drive Trigger"].data.file_id }}`
- **Output:** Binary file data

**Purpose:** Download the image file content

---

### **Node 4: Convert Image to 64x64 Array**
**Type:** Execute Command / Function

**Implementation Options:**

#### **Option A: Python Script (Recommended)**
**Type:** Execute Command

```bash
python3 convert_image.py --input /tmp/input_image.jpg --output /tmp/frame.json
```

**Python Script Location:** `~/convert_image.py`

**Script Logic:**
```python
#!/usr/bin/env python3
import cv2
import numpy as np
import json
import sys
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--input', required=True, help='Input image path')
parser.add_argument('--output', required=True, help='Output JSON path')
args = parser.parse_args()

# Load and prepare image
img = cv2.imread(args.input)
img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

# Resize to 64x64 with aspect ratio preservation
h, w, _ = img.shape
scale = min(64 / w, 64 / h)
new_w = int(w * scale)
new_h = int(h * scale)

resized = cv2.resize(img, (new_w, new_h), interpolation=cv2.INTER_AREA)

canvas = np.zeros((64, 64, 3), dtype=np.uint8)
x_offset = (64 - new_w) // 2
y_offset = (64 - new_h) // 2
canvas[y_offset:y_offset+new_h, x_offset:x_offset+new_w] = resized

# Generate hex array
frame_array = []
for y in range(64):
    x_range = range(63, -1, -1) if (y % 2 == 1) else range(64)  # Zig-zag
    for x in x_range:
        r, g, b = map(int, canvas[y, x])
        value = (r << 16) | (g << 8) | b
        frame_array.append(f"0x{value:06X}")

# Output JSON
output = {
    "frame_data": frame_array,
    "width": 64,
    "height": 64,
    "total_pixels": 4096,
    "format": "hex_rgb"
}

with open(args.output, 'w') as f:
    json.dump(output, f)

print(json.dumps(output))
```

**Purpose:** Convert image to 64x64 RGB hex array

---

### **Node 5: Update Processed File ID**
**Type:** Set Variable / Object

**Configuration:**
```json
{
  "last_processed_file_id": "{{ $node['Google Drive Trigger'].data.file_id }}",
  "timestamp": "{{ new Date().toISOString() }}",
  "filename": "{{ $node['Google Drive Trigger'].data.name }}"
}
```

**Purpose:** Track that this file has been processed

---

### **Node 6: Store Frame Data**
**Type:** HTTP Request (POST) OR Database

#### **Option A: Store in n8n Database**
**Type:** Function Node

```javascript
// Store frame data in n8n memory/database
return {
  frame_id: {{ Math.random().toString(36).substr(2, 9) }},
  frame_data: {{ $node['Convert Image to 64x64 Array'].data.frame_data }},
  source_file: "{{ $node['Google Drive Trigger'].data.name }}",
  updated_at: new Date().toISOString(),
  version: 1
}
```

#### **Option B: Store in External Database**
**Type:** PostgreSQL / MongoDB / Supabase node

```sql
INSERT INTO frame_data (frame_array, source_file, updated_at, version)
VALUES (%s, %s, NOW(), 1)
ON CONFLICT (id) DO UPDATE
SET frame_array = EXCLUDED.frame_array, updated_at = NOW(), version = version + 1
RETURNING *;
```

**Purpose:** Persist the latest frame array for API access

---

### **Node 7: Expose REST API Endpoint**
**Type:** Webhook (Custom) - HTTP Server

**Setup in n8n:**
- Create a **Webhook** node (Receive / POST)
- **URL:** `https://your-n8n-instance.com/webhook/frame/latest`
- **Method:** GET
- **Response:** Return stored frame data as JSON

**Response Format:**
```json
{
  "frame_data": ["0xFFFFFF", "0x000000", ...],
  "version": 1,
  "source_file": "image.jpg",
  "updated_at": "2026-04-14T12:34:56.789Z",
  "width": 64,
  "height": 64,
  "format": "hex_rgb"
}
```

#### **Alternative: Use n8n Built-in Webhook**
**Type:** Webhook (GET)

**Trigger Condition:** On GET request

**Response:** 
```javascript
{
  statusCode: 200,
  body: {
    frame_data: $node['Store Frame Data'].data.frame_data,
    version: $node['Store Frame Data'].data.version,
    updated_at: $node['Store Frame Data'].data.updated_at
  }
}
```

---

## Complete Workflow Node Sequence

```
┌─────────────────────────────────────────────┐
│ Node 1: Google Drive Trigger (Poll)         │
│ Scans LED_Images folder every 2 min         │
└──────────────┬──────────────────────────────┘
               │
┌──────────────▼──────────────────────────────┐
│ Node 2: IF - New/Unprocessed Image?         │
└──────────────┬──────────────────────────────┘
               │ YES
┌──────────────▼──────────────────────────────┐
│ Node 3: Download Image from Drive           │
└──────────────┬──────────────────────────────┘
               │
┌──────────────▼──────────────────────────────┐
│ Node 4: Execute Python Script               │
│ convert_image.py (resize to 64x64)          │
└──────────────┬──────────────────────────────┘
               │
┌──────────────▼──────────────────────────────┐
│ Node 5: Update Processed File ID            │
└──────────────┬──────────────────────────────┘
               │
┌──────────────▼──────────────────────────────┐
│ Node 6: Store Frame Data                    │
│ (Save latest array + metadata)              │
└─────────────────────────────────────────────┘

┌─────────────────────────────────────────────┐
│ Node 7: Webhook - GET /webhook/frame/latest │
│ ← ESP32 polls every 30 seconds              │
└─────────────────────────────────────────────┘
```

---

## ESP32 Integration

### ESP32 Code (Polling Every 30 Seconds)

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define POLL_INTERVAL 30000  // 30 seconds

unsigned long lastPoll = 0;
uint32_t currentFrameVersion = 0;

void fetchLatestFrame() {
  if (millis() - lastPoll < POLL_INTERVAL) {
    return;
  }
  
  HTTPClient http;
  String url = "https://your-n8n-instance.com/webhook/frame/latest";
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    
    // Parse JSON
    DynamicJsonDocument doc(262144);  // 256KB buffer for 4096 pixels
    deserializeJson(doc, payload);
    
    uint32_t version = doc["version"];
    
    // Only update if new version
    if (version > currentFrameVersion) {
      currentFrameVersion = version;
      
      // Extract frame data
      JsonArray frameArray = doc["frame_data"];
      
      // Display on LED matrix
      for (int i = 0; i < 4096; i++) {
        uint32_t color = strtol(frameArray[i], NULL, 0);
        // dma_display->drawPixel(..., color);
      }
    }
  }
  
  http.end();
  lastPoll = millis();
}

void loop() {
  fetchLatestFrame();
  delay(1000);
}
```

---

## Configuration Checklist

- [ ] **Google Drive Folder:** Create/specify folder name (e.g., `LED_Images`)
- [ ] **n8n Instance:** Self-hosted or cloud (ngrok for local)
- [ ] **Python Script:** Save to accessible path (e.g., `/root/scripts/convert_image.py`)
- [ ] **Webhook URL:** Make publicly accessible or on local network
- [ ] **ESP32 WiFi:** Configure SSID & Password
- [ ] **API Endpoint:** Test with `curl https://your-n8n/webhook/frame/latest`

---

## Error Handling

Add error nodes for:
1. **File download fails** → Log error, retry in 5 min
2. **Image conversion fails** → Keep previous frame, alert user
3. **API not responding** → Return cached frame
4. **Unsupported file type** → Skip and continue monitoring

---

## Performance Considerations

- **Poll Interval:** 2 minutes for Drive scanning (avoid rate limits)
- **ESP32 Poll Interval:** 30 seconds (configurable)
- **Frame Storage:** Store only 1 version (latest)
- **Image Compression:** Consider WebP for smaller downloads
- **Cache:** ESP32 stores frame in memory, only updates on version change

---

## Folder Structure (Google Drive)

```
My Drive
└── LED_Images/
    ├── logo.png
    ├── animation_frame1.jpg
    ├── animation_frame2.jpg
    └── current_display.png  (← Latest will be processed)
```

---

## Testing Workflow

1. Upload test image to `LED_Images` folder
2. Wait 2 minutes (poll interval)
3. Check n8n execution logs
4. Call API endpoint: `GET https://your-n8n/webhook/frame/latest`
5. Verify response contains 4096 hex color values
6. ESP32 fetches and displays frame

---

## Next Steps

1. Create n8n workflow with nodes above
2. Deploy Python conversion script
3. Test with sample images
4. Update ESP32 client code with webhook URL
5. Deploy and monitor

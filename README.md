# He thong kiem soat moi truong vuon uom - ESP32-S3 (ESP-IDF v5.4)

## Cau truc thu muc (chep vao thu muc `main/` cua project)
| File | Chuc nang |
|---|---|
| `do_an_thiet_ke_he_thong_nhung.cpp` | Chuong trinh chinh: vong lap doc cam bien, dieu khien, hien thi |
| `config.h` | Chan ket noi, nguong dieu khien, hieu chinh cam bien (chi sua o day) |
| `control.h` | Logic dieu khien bom (tuoi theo xung) va quat (co do tre) |
| `sensors.h/.cpp` | Doc DHT22, BH1750, cam bien do am dat |
| `actuators.h/.cpp` | Dieu khien relay bom va quat |
| `oled_ssd1306.h` | Driver OLED 0.96 inch (SSD1306, I2C) |
| `CMakeLists.txt`, `idf_component.yml` | Khai bao file nguon va thu vien (dht, bh1750) |

CMakeLists.txt o thu muc goc project giu nguyen.

## Build va nap
    idf.py set-target esp32s3      (chi lan dau)
    idf.py build
    idf.py -p COMx flash monitor

## Cac buoc kiem tra truoc khi chay that
1. Hieu chinh cam bien do am dat: xem gia tri `ADC raw` tren Serial khi de cam bien ngoai khong khi
   (-> SOIL_RAW_DRY) va khi nhung nuoc toi vach gioi han (-> SOIL_RAW_WET), sua trong config.h.
2. Kiem tra relay: neu bom/quat chay luc dang le phai tat, doi RELAY_ACTIVE_LOW (1 <-> 0).
3. Kiem tra dia chi OLED: neu log bao khong khoi tao duoc OLED, doi OLED_I2C_ADDR sang 0x3D.
4. Lan dau nen thu voi bom chua noi nuoc (chi nghe tieng relay) de xem logic tuoi.

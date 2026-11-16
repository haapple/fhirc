# IoT on FHIR 轉檔上傳與醫護查詢系統操作說明書 (Quick Start)

* **核心程式**：`main.c`（邊緣端 CSV 轉譯與 HTTP 上傳）、`index.html`（醫護端查詢與管理網頁）
* **適用環境**：Windows 10 / 11 + Visual Studio 2022 + 現代瀏覽器（Chrome / Edge）
* **系統功能**：讀取本地生理量測 CSV 檔，以純 C 語言轉成國際標準 HL7 FHIR R4 Observation JSON 並上傳至 HAPI-FHIR 伺服器，再透過網頁介面進行即時調閱、數值修訂（PUT）與管理。

---

### 一、 檔案清單（專案資料夾裡有哪些檔案？）

請確認專案目錄內包含以下檔案：

1. **`main.c`**：C 語言核心轉譯與 HTTP POST 上傳程式（無 `malloc`，使用固定緩衝區）。
2. **`vital_signs.csv`**：模擬生理感測設備輸出的測試數據清單。
3. **`index.html`**：醫護端資料查詢、數值編輯（CRUD）與 CSV 驗證網頁。
4. **`obs_temp_*.json`**：執行 `main.c` 後自動產出的本機暫存快取檔（模擬斷網緩存）。

> **位置提醒**：`vital_signs.csv` 必須放在跟 `main.c`（以及 Visual Studio 的 `.vcxproj` 專案檔）在**同一個資料夾**，否則程式執行時會報 `[ERROR] Cannot open vital_signs.csv`。

---

### 二、 CSV 資料準備格式 (`vital_signs.csv`)

打開 `vital_signs.csv`，確認內容為逗點分隔格式（第一行為標題，第一欄務必填寫伺服器已建立的真實病患 ID）：

```csv
PatientID,Timestamp,HeartRate,SystolicBP,DiastolicBP
364734,2026-10-28T08:30:00Z,72,125,82
364734,2026-10-28T12:15:00Z,80,132,85
364734,2026-10-28T09:00:00Z,65,118,78
364734,2026-10-28T14:20:00Z,88,140,90
364734,2026-10-28T18:45:00Z,75,122,80

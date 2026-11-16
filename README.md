
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

```

* **第 1 欄 (`PatientID`)**：填寫已在雲端建好的病患 FHIR ID（本範例為 `364734`）。
* **第 2 欄 (`Timestamp`)**：量測時間（標準 ISO 8601 格式，如 `2026-10-28T08:30:00Z`）。
* **第 3 欄 (`HeartRate`)**：心率數值（單位：`/min`）。
* **第 4 欄 (`SystolicBP`)**：收縮壓數值（單位：`mm[Hg]`）。
* **第 5 欄 (`DiastolicBP`)**：舒張壓數值（單位：`mm[Hg]`）。

---

### 三、 操作步驟 1：執行 C 語言轉檔與上傳 (`main.c`)

1. **開啟專案**：在 Visual Studio 打開包含 `main.c` 的專案。
2. **啟動程式**：按鍵盤 **`F5`**（或點擊上方綠色實心三角形「本機 Windows 偵錯工具」）。
3. **自動處理流程**：
* 畫面彈出黑色命令提示字元視窗（Console）。
* 程式逐筆解析 CSV 數據並填入 LOINC 國際編碼（`85354-9` 血壓面板、`8867-4` 心率、`8480-6` 收縮壓、`8462-4` 舒張壓）。
* 在同目錄產出本地快取檔 `obs_temp_1.json` ~ `obs_temp_5.json`。
* 呼叫 Windows 內建 `curl` 發送 HTTP POST 至 `https://hapi.fhir.org/baseR4/Observation`。


4. **執行完成**：終端機顯示 `[DONE] Processed and uploaded 5 Observation resources to cloud.`，按任意鍵即可關閉視窗。

---

### 四、 操作步驟 2：開啟醫護端網頁查詢與管理 (`index.html`)

資料透過 C 語言上傳後，可使用網頁介面模擬醫護人員進行調閱與修訂：

1. **開啟網頁**：直接雙擊 `index.html`，以 Chrome 或 Edge 瀏覽器開啟。
2. **載入測試環境**：點擊頁面右上角的 **「⚡ 一鍵載入 C 程式測試環境 (Patient/364734)」**，系統會自動綁定 `Organization/364732` 與 `Patient/364734`，並向伺服器查詢（`GET`）所有量測紀錄。
3. **檢視與增刪改查（CRUD）**：
* **查詢 (`GET`)**：表格自動列出心率、收縮壓、舒張壓明細（若收縮壓 $\ge 130$ 或舒張壓 $\ge 85$ 會自動以紅底標示提醒），點擊 **「檢視 JSON」** 可查看完整原始碼。
* **修改數值 (`PUT`)**：點擊 **「✏️ 編輯數值 (PUT)」** 可直接修改心率或血壓數字；送出後依 FHIR 規範狀態會自動變更為 `amended`（已修訂），版本號亦會從 `v1` 自動升級為 `v2`。
* **刪除 (`DELETE`)**：點擊 **「刪除 (DELETE)」** 可清除多餘的測試資料。



---

### 五、 如何確認「真的有寫入雲端伺服器」？

#### 1. 檢視終端機回傳 JSON

執行 `main.c` 時，只要伺服器回傳的內容包含 `"resourceType": "Observation"` 以及系統配發的 `"id"`，即代表寫入成功：

```json
{
  "resourceType": "Observation",
  "id": "381209",
  "meta": {
    "versionId": "1",
    "lastUpdated": "2026-11-01T22:55:35.816-05:00"
  },
  "status": "final"
}

```

#### 2. 瀏覽器直達端點驗證

將終端機印出的 `id` 接在伺服器網址後方，直接用瀏覽器開啟即可查看雲端原始資料：

* 範例網址：https://hapi.fhir.org/baseR4/Observation/381209
* **驗收標準**：畫面顯示 `HTTP 200 OK` 並完整呈現該筆病患的血壓與心率 JSON 結構。

---

### 六、 常見錯誤排查 (Troubleshooting)

| 錯誤狀況 | 原因 | 解決方法 |
| --- | --- | --- |
| `[ERROR] Cannot open vital_signs.csv` | 程式找不到 CSV 檔案 | 請將 `vital_signs.csv` 複製到跟 `main.c`（或 `.vcxproj` 專案檔）同一個資料夾。 |
| `HTTP 400 Bad Request`<br>

<br>`(Resource Patient/xxx not found)` | 伺服器找不到該病患 | CSV 第一欄不可填寫未註冊代碼（如 `P1001`），請改用伺服器已建立的真實 ID（如 `364734`）。 |
| `HTTP 404 Not Found` | 瀏覽器網址打錯或缺字 | 檢查網址列是否漏複製最後一碼（例如把 `381209` 誤打成 `38120`）。 |
| 終端機畫面出現問號亂碼 | Windows 終端機 Big5 與伺服器 UTF-8 衝突 | 程式日誌已全面改為純英文輸出，請確認使用的是最新版 `main.c`。 |
| 網頁點擊查詢後顯示「查無紀錄」 | 病患 ID 輸入錯誤或公開伺服器延遲 | 確認查詢框內填寫 `364734`，或點擊右上角「一鍵載入 C 程式測試環境」重新整理。 |

```

```

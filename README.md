

# IoT on FHIR 轉檔上傳程式操作說明書 (Quick Start)

**程式名稱**：`main.c` (CSV to FHIR Uploader)

**適用環境**：Windows 10 / 11 + Visual Studio 2022

**一句話功能**：讀取本地生理量測 CSV 檔，自動轉成國際標準 FHIR Observation JSON，並自動透過網路送進 HAPI-FHIR 伺服器。

---

### 一、 準備清單（需要哪些檔案？）

請確認專案資料夾內只有以下兩個關鍵檔案放在**同一個目錄**下：

1. **`main.c`**：主要 C 語言程式碼。


2. **`vital_signs.csv`**：要上傳的量測數據清單。



> **位置提醒**：`vital_signs.csv` 必須放在跟 `main.c`（以及 Visual Studio 的 `.vcxproj` 專案檔）在**同一個資料夾**，否則程式會報 `[ERROR] Cannot open vital_signs.csv`。
> 
> 

---

### 二、 CSV 資料準備格式

打開 `vital_signs.csv`，確認內容格式如下（逗點分隔，第一行是標題，第一欄務必填寫伺服器已存在的真實病患 ID）：

```csv
PatientID,Timestamp,HeartRate,SystolicBP,DiastolicBP
364734,2026-10-28T08:30:00Z,72,125,82
364734,2026-10-28T12:15:00Z,80,132,85
364734,2026-10-28T09:00:00Z,65,118,78
364734,2026-10-28T14:20:00Z,88,140,90
364734,2026-10-28T18:45:00Z,75,122,80

```

* **第 1 欄 (PatientID)**：填寫已在雲端建好的病患 ID（本範例為 `364734`）。


* **第 2 欄 (Timestamp)**：量測時間（標準 ISO 8601 格式，如 `2026-10-28T08:30:00Z`）。


* **第 3 欄 (HeartRate)**：心率數值。


* **第 4 欄 (SystolicBP)**：收縮壓數值。


* **第 5 欄 (DiastolicBP)**：舒張壓數值。



---

### 三、 操作步驟（如何執行？）

1. **開啟專案**：在 Visual Studio 打開包含 `main.c` 的專案。


2. **啟動程式**：按鍵盤 **`F5`**（或點擊上方綠色實心三角形「本機 Windows 偵錯工具」）。


3. **自動處理中**：
* 畫面會彈出黑色命令提示字元（Console）。


* 程式會逐筆讀取 CSV 數據。


* 自動在資料夾產出暫存檔 `obs_temp_1.json`、`obs_temp_2.json`... 等。


* 自動透過 Windows 內建 curl 將 JSON 送到 HAPI-FHIR 伺服器。




4. **結束**：黑色視窗顯示 `[DONE] Processed and uploaded 5 Observation resources to cloud.`，按任意鍵即可關閉視窗。



---

### 四、 如何確認「真的有上傳成功」？

#### 1. 看黑色視窗回傳的字串

當看到伺服器印出一大串 JSON，開頭長這樣，就代表成功寫入資料庫了：

```json
{
  "resourceType": "Observation",
  "id": "381209",
  "meta": {
    "versionId": "1",
    "lastUpdated": "2026-11-01T22:55:35.816-05:00"
  },
  "status": "final",
  ...
}

```

* **關鍵指標**：只要有出現 `"resourceType": "Observation"` 和伺服器指派的 `"id"`（例如 `381209`），就代表上傳成功。



#### 2. 直接用瀏覽器連線檢查

複製終端機印出來的 ID，打開 Chrome 或 Edge 瀏覽器，輸入：
`[https://hapi.fhir.org/baseR4/Observation/剛剛產生的ID](https://hapi.fhir.org/baseR4/Observation/剛剛產生的ID)`

* 範例網址：`[https://hapi.fhir.org/baseR4/Observation/381209](https://hapi.fhir.org/baseR4/Observation/381209)`


* **驗收結果**：畫面上能直接看到包含收縮壓 125、舒張壓 82、心率 72 的完整 JSON，證實資料已存在於國際公開資料庫中。



---

### 五、 常見錯誤排查（Troubleshooting）

| 錯誤狀況 | 原因 | 解決方法 |
| --- | --- | --- |
| **`[ERROR] Cannot open vital_signs.csv`**

 | 程式找不到檔案。

 | 請將 `vital_signs.csv` 移動到跟 `main.c` / `.vcxproj` 同一個資料夾。

 |
| **`HTTP 400 Bad Request`**<br>

<br>(`Resource Patient/xxx not found`) | 伺服器找不到該病患。 | CSV 第一欄不要寫 `P1001` 等未註冊代碼，請改成在伺服器上真實存在的 ID（如 `364734`）。

 |
| **`HTTP 404 Not Found`**

 | 瀏覽器網址打錯或少字。

 | 檢查網址列是否少複製了數字（例如把 `381209` 漏打成 `38120`）。

 |
| **畫面噴一堆問號亂碼** | 終端機編碼與回傳文字衝突。 | 本版本程式日誌已全面改為純英文顯示，請直接使用最新版 `main.c` 即可避免亂碼。

 |

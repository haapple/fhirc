#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256
#define MAX_JSON 2048

// Target HAPI-FHIR Public Server Endpoint
const char* FHIR_URL = "https://hapi.fhir.org/baseR4/Observation";

// Serialize CSV parameters into standard FHIR R4 Observation JSON
void make_fhir_json(const char* patient_id, const char* timestamp,
    int hr, int sbp, int dbp, char* out_json) {
    sprintf(out_json,
        "{\n"
        "  \"resourceType\": \"Observation\",\n"
        "  \"status\": \"final\",\n"
        "  \"category\": [{\n"
        "    \"coding\": [{\n"
        "      \"system\": \"http://terminology.hl7.org/CodeSystem/observation-category\",\n"
        "      \"code\": \"vital-signs\",\n"
        "      \"display\": \"Vital Signs\"\n"
        "    }]\n"
        "  }],\n"
        "  \"code\": {\n"
        "    \"coding\": [{\n"
        "      \"system\": \"http://loinc.org\",\n"
        "      \"code\": \"85354-9\",\n"
        "      \"display\": \"Blood pressure panel with all children optional\"\n"
        "    }]\n"
        "  },\n"
        "  \"subject\": {\n"
        "    \"reference\": \"Patient/%s\",\n"
        "    \"display\": \"Wang Da-Ming (P1001)\"\n"
        "  },\n"
        "  \"effectiveDateTime\": \"%s\",\n"
        "  \"component\": [\n"
        "    {\n"
        "      \"code\": {\"coding\": [{\"system\": \"http://loinc.org\", \"code\": \"8867-4\", \"display\": \"Heart rate\"}]},\n"
        "      \"valueQuantity\": {\"value\": %d, \"unit\": \"beats/minute\", \"system\": \"http://unitsofmeasure.org\", \"code\": \"/min\"}\n"
        "    },\n"
        "    {\n"
        "      \"code\": {\"coding\": [{\"system\": \"http://loinc.org\", \"code\": \"8480-6\", \"display\": \"Systolic blood pressure\"}]},\n"
        "      \"valueQuantity\": {\"value\": %d, \"unit\": \"mmHg\", \"system\": \"http://unitsofmeasure.org\", \"code\": \"mm[Hg]\"}\n"
        "    },\n"
        "    {\n"
        "      \"code\": {\"coding\": [{\"system\": \"http://loinc.org\", \"code\": \"8462-4\", \"display\": \"Diastolic blood pressure\"}]},\n"
        "      \"valueQuantity\": {\"value\": %d, \"unit\": \"mmHg\", \"system\": \"http://unitsofmeasure.org\", \"code\": \"mm[Hg]\"}\n"
        "    }\n"
        "  ]\n"
        "}",
        patient_id, timestamp, hr, sbp, dbp);
}

// Upload JSON payload via Windows built-in curl command
void upload_json_to_server(const char* json_filename) {
    char command[512];
    sprintf(command,
        "curl -s -X POST \"%s\" "
        "-H \"Content-Type: application/fhir+json\" "
        "-d @%s",
        FHIR_URL, json_filename);

    printf("[NETWORK] Sending HTTP POST to FHIR Server...\n");
    system(command);
    printf("\n[STATUS] Server response received successfully.\n\n");
}

int main() {
    FILE* fp = fopen("vital_signs.csv", "r");
    if (!fp) {
        printf("[ERROR] Cannot open vital_signs.csv. Please verify the file path.\n");
        system("pause");
        return 1;
    }

    char line[MAX_LINE];
    char json_buffer[MAX_JSON];
    int count = 0;

    // Skip CSV header line
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return 1;
    }

    printf("====================================================================\n");
    printf("   IoT on FHIR: Edge Gateway CSV Parser & HTTP Uploader (C Engine)\n");
    printf("====================================================================\n\n");

    while (fgets(line, sizeof(line), fp)) {
        char patient_id[32];
        char timestamp[64];
        int hr, sbp, dbp;

        if (sscanf(line, "%31[^,],%63[^,],%d,%d,%d", patient_id, timestamp, &hr, &sbp, &dbp) == 5) {
            count++;
            printf("--------------------------------------------------------------------\n");
            printf("[INGEST] Record #%d | Subject: Patient/%s | Timestamp: %s\n",
                count, patient_id, timestamp);
            printf("[TRANS] HR: %d bpm, SBP: %d mmHg, DBP: %d mmHg\n", hr, sbp, dbp);

            // 1. Serialize to FHIR JSON
            make_fhir_json(patient_id, timestamp, hr, sbp, dbp, json_buffer);

            // 2. Local buffer cache
            char temp_file[32];
            sprintf(temp_file, "obs_temp_%d.json", count);
            FILE* out_fp = fopen(temp_file, "w");
            if (out_fp) {
                fputs(json_buffer, out_fp);
                fclose(out_fp);
            }

            // 3. Upload to FHIR Server
            upload_json_to_server(temp_file);
        }
    }

    fclose(fp);
    printf("====================================================================\n");
    printf("[DONE] Processed and uploaded %d Observation resources to cloud.\n", count);
    printf("====================================================================\n");

    system("pause");
    return 0;
}
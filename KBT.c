#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h> // Untuk toupper

// Fungsi untuk mengonversi string ke huruf kapital
void toUpperCase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper(str[i]);
    }
}

// Fungsi untuk mendapatkan indeks terminal
int getTerminalIndex(const char *terminal_name) {
    char terminals[14][30] = {
        "MEDAN", "PEMATANG SIANTAR", "PARAPAT", "PORSEA", "BALIGE", "SIBORONG-BORONG",
        "TARUTUNG", "PAHAE", "SIPIROK", "PADANG SIDIMPUAN", "PENYABUNGAN", "PAKKAT",
        "TANTOM", "SAMOSIR"
    };

    char temp_name[30];
    strcpy(temp_name, terminal_name);
    toUpperCase(temp_name); // Konversi input ke uppercase

    for (int i = 0; i < 14; i++) {
        if (strcmp(temp_name, terminals[i]) == 0) {
            return i;
        }
    }
    return -1; // Tidak ditemukan
}

// Fungsi untuk menghitung harga berdasarkan rute
long calculateRoutePrice(int from_index, int to_index) {
    if (from_index == -1 || to_index == -1) {
        return -1; // Rute tidak valid
    }
    
    // Asumsi harga dasar adalah 0, lalu ditambahkan per "langkah"
    // Jarak dihitung absolut dari indeks
    int distance = from_index - to_index;
    if (distance < 0) distance *= -1; // Ambil nilai absolut
    
    // Setiap "langkah" (selisih indeks) menambah 15.000
    return distance * 15000;
}

// Fungsi untuk memvalidasi dan menyesuaikan jam keberangkatan
void adjustDepartureTime(int *hour, int *minute, int dayOfWeek) {
    bool is_weekend = (dayOfWeek == 0 || dayOfWeek == 6); // Minggu = 0, Sabtu = 6

    int target_hour = *hour;
    int target_minute = *minute;

    int valid_slots[6][2]; // Max 6 slot: start_hour, start_minute
    int num_slots = 0;

    if (!is_weekend) { // Senin - Jumat
        valid_slots[0][0] = 6; valid_slots[0][1] = 0;
        valid_slots[1][0] = 11; valid_slots[1][1] = 0;
        valid_slots[2][0] = 12; valid_slots[2][1] = 30;
        valid_slots[3][0] = 17; valid_slots[3][1] = 0;
        valid_slots[4][0] = 18; valid_slots[4][1] = 30;
        valid_slots[5][0] = 2; valid_slots[5][1] = 0; // Ini untuk jam 02:00 keesokan harinya
        num_slots = 6;
    } else { // Weekend
        valid_slots[0][0] = 13; valid_slots[0][1] = 30;
        valid_slots[1][0] = 17; valid_slots[1][1] = 0;
        valid_slots[2][0] = 18; valid_slots[2][1] = 30;
        valid_slots[3][0] = 0; valid_slots[3][1] = 0; // Ini untuk jam 00:00 keesokan harinya
        num_slots = 4;
    }

    // Konversi jam target ke menit total dari 00:00
    int total_target_minutes = target_hour * 60 + target_minute;
    if (target_hour >= 0 && target_hour <= 2) { // Untuk jam 00:00 - 02:00, anggap sebagai hari berikutnya jika perlu
         // Ini tricky, untuk slot 02:00 atau 00:00 di weekend, diasumsikan user pesan di hari sebelumnya
         // Jadi kita cek jika jam target ada di range malam, akan dianggap sebagai hari yang sama
         // Tapi jika user input 01:00, itu tetap 01:00. Logika "terdekat" perlu memperhitungkan ini
    }

    int best_slot_hour = -1, best_slot_minute = -1;
    long min_diff = 24 * 60 * 60; // Inisialisasi dengan nilai besar (detik)

    for (int i = 0; i < num_slots; i += 2) { // Iterasi setiap pasang slot (start_hour, start_minute) dan (end_hour, end_minute)
        int start_h = valid_slots[i][0];
        int start_m = valid_slots[i][1];
        int end_h = valid_slots[i+1][0];
        int end_m = valid_slots[i+1][1];

        int current_slot_start_minutes = start_h * 60 + start_m;
        int current_slot_end_minutes = end_h * 60 + end_m;
        
        // Handle slot yang melewati tengah malam (misal 18:30 - 02:00)
        if (end_h < start_h) { // Contoh: 18:30 sampai 02:00
            current_slot_end_minutes += 24 * 60; // Tambah 24 jam untuk melewati tengah malam
            if (total_target_minutes <= end_h * 60 + end_m) { // Jika target jam di awal hari (00:00 - 02:00)
                total_target_minutes += 24 * 60; // Anggap target jam juga di hari berikutnya
            }
        }
        
        // Cek apakah jam target berada di dalam slot
        if (total_target_minutes >= current_slot_start_minutes && total_target_minutes <= current_slot_end_minutes) {
            *hour = target_hour;
            *minute = target_minute;
            return; // Jam sudah valid dalam slot, tidak perlu disesuaikan
        }

        // Jika tidak di dalam slot, cari yang terdekat
        // Jarak ke awal slot
        long diff_to_start = (long)total_target_minutes - current_slot_start_minutes;
        if (diff_to_start < 0) diff_to_start = -diff_to_start;
        
        // Jarak ke akhir slot
        long diff_to_end = (long)total_target_minutes - current_slot_end_minutes;
        if (diff_to_end < 0) diff_to_end = -diff_to_end;

        // Pilih yang terdekat dari awal atau akhir slot
        long current_min_diff_slot;
        int temp_hour, temp_minute;

        if (total_target_minutes < current_slot_start_minutes) { // Target sebelum slot
            current_min_diff_slot = (long)current_slot_start_minutes - total_target_minutes;
            temp_hour = start_h;
            temp_minute = start_m;
        } else { // Target setelah slot
            current_min_diff_slot = (long)total_target_minutes - current_slot_end_minutes;
            temp_hour = end_h;
            temp_minute = end_m;
        }

        if (current_min_diff_slot < min_diff) {
            min_diff = current_min_diff_slot;
            best_slot_hour = temp_hour;
            best_slot_minute = temp_minute;
        }
    }
    
    // Jika tidak ada slot yang pas, ambil slot pertama atau terdekat dari keseluruhan range
    if (best_slot_hour == -1) { // Ini seharusnya tidak terjadi jika ada slot
        *hour = valid_slots[0][0];
        *minute = valid_slots[0][1];
    } else {
        *hour = best_slot_hour;
        *minute = best_slot_minute;
    }
}


int main() {
    char nama[100];
    int dewasa, remaja, anak;
    char dari_str[30], tujuan_str[30];
    int tanggal_d, tanggal_m, tanggal_y;
    int jam_h, jam_m;
    int nomor_mobil;
    long harga_tiket_dasar; // Harga berdasarkan tipe mobil
    long harga_rute;        // Harga berdasarkan rute
    long total_harga;
    
    // Array nama terminal untuk tampilan
    char terminals_display[14][30] = {
        "Medan", "Pematang Siantar", "Parapat", "Porsea", "Balige", "Siborong-borong",
        "Tarutung", "Pahae", "Sipirok", "Padang Sidimpuan", "Penyabungan", "Pakkat",
        "Tantom", "Samosir"
    };

    printf("=======================================\n");
    printf("     SISTEM TICKETING MOBIL KBT        \n");
    printf("=======================================\n\n");

    printf("Masukkan Nama Pemesan: ");
    fgets(nama, sizeof(nama), stdin);
    nama[strcspn(nama, "\n")] = 0; // Hapus newline

    printf("Jumlah Penumpang (Dewasa, Remaja, Anak):\n");
    printf("  Dewasa: ");
    scanf("%d", &dewasa);
    printf("  Remaja: ");
    scanf("%d", &remaja);
    printf("  Anak: ");
    scanf("%d", &anak);
    getchar(); // Konsumsi newline setelah scanf

    // Tampilkan daftar terminal yang tersedia
    printf("\n--- Daftar Terminal ---\n");
    for (int i = 0; i < 14; i++) {
        printf("%d. %s\n", i + 1, terminals_display[i]);
    }
    printf("------------------------\n");

    int dari_index = -1, tujuan_index = -1;
    while (dari_index == -1) {
        printf("Dari Terminal (e.g., Medan): ");
        fgets(dari_str, sizeof(dari_str), stdin);
        dari_str[strcspn(dari_str, "\n")] = 0;
        dari_index = getTerminalIndex(dari_str);
        if (dari_index == -1) {
            printf("Terminal asal tidak valid. Mohon masukkan nama terminal yang benar.\n");
        }
    }

    while (tujuan_index == -1 || tujuan_index == dari_index) {
        printf("Ke Terminal (e.g., Samosir): ");
        fgets(tujuan_str, sizeof(tujuan_str), stdin);
        tujuan_str[strcspn(tujuan_str, "\n")] = 0;
        tujuan_index = getTerminalIndex(tujuan_str);
        if (tujuan_index == -1) {
            printf("Terminal tujuan tidak valid. Mohon masukkan nama terminal yang benar.\n");
        } else if (tujuan_index == dari_index) {
            printf("Terminal tujuan tidak boleh sama dengan terminal asal. Mohon pilih terminal yang berbeda.\n");
        }
    }
    
    harga_rute = calculateRoutePrice(dari_index, tujuan_index);
    if (harga_rute == -1) {
        printf("Terjadi kesalahan dalam perhitungan harga rute.\n");
        return 1; // Keluar program jika ada error
    }

    printf("Tanggal Berangkat (DD MM YYYY): ");
    scanf("%d %d %d", &tanggal_d, &tanggal_m, &tanggal_y);
    getchar(); // Konsumsi newline

    // Dapatkan hari dalam seminggu untuk menyesuaikan jam
    struct tm t = {0};
    t.tm_year = tanggal_y - 1900;
    t.tm_mon = tanggal_m - 1;
    t.tm_mday = tanggal_d;
    mktime(&t); // Mengisi tm_wday

    printf("Jam Berangkat (HH MM, format 24 jam): ");
    scanf("%d %d", &jam_h, &jam_m);
    getchar(); // Konsumsi newline
    
    // Sesuaikan jam keberangkatan
    int original_h = jam_h;
    int original_m = jam_m;
    adjustDepartureTime(&jam_h, &jam_m, t.tm_wday);
    if (original_h != jam_h || original_m != jam_m) {
        printf("Jam %02d:%02d tidak tersedia. Jam disesuaikan ke waktu terdekat: %02d:%02d.\n", original_h, original_m, jam_h, jam_m);
    }


    printf("Nomor Mobil (1-20): ");
    scanf("%d", &nomor_mobil);
    getchar(); // Konsumsi newline

    // Hitung harga dasar berdasarkan nomor mobil
    if (nomor_mobil >= 1 && nomor_mobil <= 10) {
        harga_tiket_dasar = 100000; // Tipe Eksklusif
    } else if (nomor_mobil >= 11 && nomor_mobil <= 20) {
        harga_tiket_dasar = 70000;  // Tipe Reguler
    } else {
        printf("Nomor mobil tidak valid. Harga standar akan digunakan (Reguler: 70.000).\n");
        harga_tiket_dasar = 70000; // Default jika nomor mobil tidak valid
    }

    total_harga = (long)(dewasa + remaja + anak) * (harga_tiket_dasar + harga_rute);

    printf("\n=======================================\n");
    printf("          TIKET PERJALANAN KBT         \n");
    printf("=======================================\n");
    printf("Nama Pemesan     : %s\n", nama);
    printf("Jumlah Penumpang : Dewasa: %d, Remaja: %d, Anak: %d\n", dewasa, remaja, anak);
    printf("Dari             : %s\n", terminals_display[dari_index]);
    printf("Tujuan           : %s\n", terminals_display[tujuan_index]);
    printf("Tanggal Berangkat: %02d-%02d-%d\n", tanggal_d, tanggal_m, tanggal_y);
    printf("Jam Berangkat    : %02d:%02d\n", jam_h, jam_m);
    printf("Nomor Mobil      : %d (Tipe %s)\n", nomor_mobil, (nomor_mobil >=1 && nomor_mobil <= 10) ? "Eksklusif" : "Reguler");
    printf("Harga Tiket per Orang:\n");
    printf("  Harga Mobil    : Rp %ld\n", harga_tiket_dasar);
    printf("  Harga Rute     : Rp %ld\n", harga_rute);
    printf("  Total Per Orang: Rp %ld\n", harga_tiket_dasar + harga_rute);
    printf("---------------------------------------\n");
    printf("TOTAL HARGA      : Rp %ld\n", total_harga);
    printf("=======================================\n");

    return 0;
}

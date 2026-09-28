# Hack VM Translator (Nand2Tetris - Project 7 & 8)
Bu proje, Nand2Tetris kursunun/kitabının 7. ve 8. projeleri kapsamında geliştirilmiş, yığın (stack) tabanlı VM (Virtual Machine) komutlarını Hack işlemcisinin doğrudan çalıştırabileceği Assembly diline (.asm) çeviren masaüstü konsol aracıdır.

# 🚀 Projenin Amacı
Yüksek seviyeli diller ile donanım mimarisi arasındaki ara katmanı oluşturan Sanal Makine mantığını kavramak amacıyla tasarlanmıştır. Bu araç; yığın aritmetiğini, bellek segmentlerini (local, argument, this, that, vb.), dallanma komutlarını (label, goto, if-goto) ve fonksiyon çağrı mimarisini (function, call, return) Hack assembly diline simüle eder.

# 🛠️ Kullanılan Teknolojiler ve Araçlar
* Programlama Dili: C# (.NET)
* Veri Yapıları: String manipülasyonları, StreamReader/StreamWriter dosya akışları.
*Mimari: Modüler ayrıştırma (Parsing) ve Assembly kod üretim sınıfı (Code Generation) ayrımı.

# ⚙️ Nasıl Çalışır? (Mimarinin İşleyişi)
VM Translator, kaynak .vm dosyalarını işlerken sırasıyla şu aşamalardan geçer:

Parser (Ayrıştırma): Dosyadaki boşlukları ve yorum satırlarını (//) temizler. Her satırı okuyarak komut türünü (C_PUSH, C_POP, C_ARITHMETIC, C_LABEL, C_FUNCTION, vb.) ve argümanlarını (arg1, arg2) ayıklar.

CodeWriter (Kod Üreticisi):

Aritmetik/Mantık: add, sub, eq, gt gibi işlemleri RAM üzerindeki yığın (stack pointer) yardımıyla low-level assembly kodlarına dönüştürür.

Bellek Segmentleri: push ve pop komutlarını; local, argument, this, that, pointer, temp ve static segmentlerine uygun pointer aritmetiğiyle RAM adreslerine mapler.

Program Kontrolü: label, goto ve if-goto komutlarıyla kod akışını yönlendiren etiketler üretir.

Fonksiyon Yönetimi: call ve return mekanizmalarında çerçeve işaretçilerini (FRAME, LCL, ARG, THIS, THAT) ve dönüş adreslerini (return address) yığında saklayarak hiyerarşik alt program çağrılarını yönetir.

```text
📂 Proje Yapısı
Plaintext
VmTranslator/
│
├── Program.cs          # Giriş noktası (Dosya/Klasör argüman yönetimi ve döngü)
├── Parser.cs           # VM dosyasını satır satır okuyup komutları ayıran sınıf
├── CodeWriter.cs       # Hack Assembly kodunu üreten ve label yönetimini yapan sınıf
└── README.md           # Proje dokümantasyonu

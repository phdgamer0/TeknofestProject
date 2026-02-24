
# Proje Katkı Rehberi

Bu doküman, projeye katkı sağlayacak **tasarımcılar, elektrik ekibi ve yazılımcılar** için hazırlanmıştır.
Lütfen dikkatlice okuyun. Yanlış bir işlem tüm repoyu bozabilir.

---

# Kod Yazmayan Katılımcılar İçin (Asset Ekleyenler)

Eğer sadece dosya (asset) ekleyecekseniz, kod tarafına dokunmanıza gerek yok.

## Klasörler

* `struct-assets/` → Mekanik / yapısal dosyalar
* `electrical-assets/` → Elektrik ile ilgili dosyalar

Lütfen dosyanızı doğru klasöre ekleyin.

---

## GitHub Üzerinden Dosya Ekleme

1. GitHub’da repoya girin
2. İlgili klasöre girin (`struct-assets` veya `electrical-assets`)
3. Sağ üstte **"Add file"** → **"Upload files"**
4. Dosyanızı sürükleyin
5. Altta açıklama yazın (örnek: `motor_driver_v2 eklendi`)
6. **Commit changes** butonuna basın

Hepsi bu kadar 👍

---

## Dosya Güncelleme

Eğer bir dosyayı güncelleyecekseniz:

* Aynı dosya adını kullanın
* Açıklamaya neyi değiştirdiğinizi yazın

  * ❌ `update`
  * ✅ `motor_driver bağlantı şeması düzeltildi`

---

## Dosya Silme (DİKKAT)

Bir dosyayı silmeden önce:

* %100 emin olun
* Başkasının dosyası olup olmadığını kontrol edin
* Emin değilsen **silme**

⚠️ Yanlış silinen dosyalar projeyi bozabilir.

---

## Repo Hijyen Kuralları

* Bilmediğiniz klasörleri silmeyin
* Kod dosyalarına dokunmayın
* Gereksiz test dosyası yüklemeyin
* Aynı dosyanın 15 versiyonunu yüklemeyin (`final_final_v3_son_artık_son.dwg` yapmayın 🙃)

---

# Yazılımcılar İçin

## İlk Kurulum (Fork & Clone)

### Repo Forklama

GitHub’da:

* Sağ üstte **Fork** butonuna basın
* Kendi hesabınıza kopyalayın

---

### Clone Etme

Git Bash veya VSCode Terminal açın:

```bash
git clone REPO_LINK
cd REPO_ADI
```

---

## Çalışma Klasörleri

Yazılımcılar için önemli klasörler:

* `yazdan-backend/`
* `val-ai/`
* `sah-frontend/`

Terminalde geçmek için:

```bash
cd val-ai
```

veya

```bash
cd sah-frontend
```

Üst klasöre dönmek için:

```bash
cd ..
```

---

## Güncel Tutmak (Pull)

Main branch güncellendiyse:

```bash
git pull origin main
```

Kod yazmadan önce **pull yapın**.
Yoksa conflict çıkabilir.

---

## Commit ve Push

Dosya eklediniz veya değiştirdiniz:

```bash
git add .
git commit -m "Kısa ve açıklayıcı mesaj yaz"
git push origin main
```

Commit mesajı örnekleri:

* ✅ `hedef targeting algoritması optimize edildi`
* ❌ `asd`
* ❌ `update`

---

## ⚠️ ÖNEMLİ KURALLAR

* Bilmediğiniz kodu silmeyin
* Başkasının klasörünü temizlemeye çalışmayın
* `.env` dosyasını repoya pushlamayın
* API key veya gizli bilgileri yüklemeyin

---

# 🛑 GENEL KURALLAR (HERKES İÇİN)

* Anlamadığınız şeyi silmeyin
* Projeyi “temizleyeyim” diyerek dosya kaldırmayın
* Her commit açıklayıcı olsun
* Repo düzenli kalsın

Bu proje ekip işidir.
Bir kişinin yanlış hamlesi herkesi etkiler.

---

# 📌 Son Not

Emin değilseniz:

> Önce sorun, sonra işlem yapın.

---

# 👥 Proje Ekibi

### **Beyza Nur Asrlan**

**GitHub:** `@beyzarsln`
Yan eksen hareket mekanizmasının tasarımı ve optimizasyonundan sorumludur. Kritik tasarım raporlarının hazırlanmasını yürütmektedir.

---

### **Yağmur Yaz**

**GitHub:** `@yagmuryaz`
Yan eksen hareketinin hassas şekilde ayarlanması ve sistem performans doğrulama süreçlerinde görev almaktadır. Teknik dokümantasyon çalışmalarına katkı sağlamaktadır.

---

### **Canay Gürsoy**

**GitHub:** `@canaygursoy`
Yükseliş (dikey) eksen hareketinin ayarlanması ve sistem stabilitesinin sağlanmasından sorumludur. Kritik tasarım ve analiz raporlarını hazırlamaktadır.

---

### **Zümra**

**GitHub:** `@Zumra61`
Sistem elektrik bağlantılarının tasarımı, kurulumu ve güvenliğinin sağlanmasından sorumludur. Elektrik altyapısının düzenli ve standartlara uygun çalışmasını koordine etmektedir.

---

### **Nazifcan Vural**

**GitHub:** `@nazifcan60`
Parça ve dış cephe çizimlerinin hazırlanması, teknik analizlerin gerçekleştirilmesi ve montaj süreçlerinin yürütülmesinden sorumludur.

---

### **Ramazan Tunçtan**

**GitHub:** `@Ramazan496`
Makinenin genel tasarımı ve işleyiş mekanizmasının geliştirilmesinden sorumludur. Gerekli mekanik parçaların tasarımı ve uygulanabilirliği üzerine çalışmalar yürütmektedir.

---

### **Muhammed Şah Kırmızı**

**GitHub:** `@muhammetsah6363-crypto`
Sistemin arayüz (UI) tasarımından ve kullanıcı deneyiminin yapılandırılmasından sorumludur.

---

### **Yazdan Samari**

**GitHub:** `@Phdgamer0`
Backend geliştirme, algoritma tasarımı ve robotik kontrol kodlarının geliştirilmesinden sorumludur. Sistem mantığının ve karar mekanizmalarının oluşturulmasını yürütmektedir.

---

### **Vall**

**GitHub:** `@gkxvall`
Görüntü işleme (Image Processing) ve yapay zeka (AI) modüllerinin geliştirilmesinden sorumludur. Algılama, analiz ve hedef tespit süreçlerini tasarlamaktadır.

---

### **EmirBurhan Özdemir**

**GitHub:** `@24060968`
Makina tasarimi, dayanıklılığı ve işleyişinden sorumlu

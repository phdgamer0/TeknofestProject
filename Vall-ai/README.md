# Görüntü İşleme Yazılım Dosyaları

Bu klasörde, proje kapsamında şu ana kadar geliştirilen **erken aşama yazılım bileşenleri** bulunmaktadır.
Kodlar aktif geliştirme sürecindedir ve **henüz final sürüm değildir**.

---

## İçerik

- [Genel Bakış](#genel-bakış)
- [Mevcut Dosyalar](#mevcut-dosyalar)
  - [object.py](#objectpy)
  - [prioritization.py](#prioritizationpy)
  - [matching.py](#matchingpy)
  - [distance_estimation.py](#distance_estimationpy)
  - [test.py](#testpy)
  - [DataPrep](#dataprep)
- [Önemli Notlar](#önemli-notlar)
- [Geliştirme Durumu](#geliştirme-durumu)

---

## Genel Bakış

Bu repodaki kodlar, hava savunma sistemi yarışması kapsamında geliştirilen bilgisayarlı görü ve hedef işleme altyapısının ilk sürümlerini içerir.
Ana amaçlar şunlardır:

- Hedef tespiti
- Hedef sınıflandırması
- Balon-hedef eşleştirmesi
- Hedef önceliklendirmesi
- Kamera tabanlı mesafe tahmini
- Kontrolcüye uygun veri paketi üretimi

---

## Mevcut Dosyalar

### `object.py`

Tespit edilen hedef/nesneye ait verileri taşıyan temel sınıfı içerir.

**Kapsadığı bilgiler:**

- Kimlik / track bilgileri
- Bounding box ve merkez noktaları
- Balon merkezi ve aim point
- Dost / düşman sınıflandırma bilgisi
- Hedef tipi bilgisi
- Güven skorları
- Öncelik skoru
- Önerilen aksiyon
- Mesafe bilgisi: `distance_m`
- Mesafe güven skoru: `distance_conf`
- Mesafe tahmin yöntemi: `distance_method`
- Kontrolcüye gönderilecek veri paketi üretimi: `to_controller_packet()`

---

### `prioritization.py`

Hedef önceliklendirme mantığını içerir.

**Değerlendirilen başlıca kriterler:**

- Hedef tipi
- Dost / düşman durumu
- Yaklaşma durumu
- Takip süresi
- Güven skoru
- Geçerli angajman bölgesinde bulunma durumu

Bu modül, aynı anda birden fazla hedef bulunduğunda hangi hedefin önce işleneceğini belirlemek için kullanılır.

---

### `matching.py`

Balon ile hedef eşleştirme algoritmasını içerir.

**Amaç:**
Tespit edilen balonun hangi hedefe ait olduğunu belirlemek ve yanlış hedef-balonu eşleştirmelerini azaltmak.

Bu modül özellikle birden fazla hedef ve balonun aynı karede bulunduğu durumlarda önemlidir.

---

### `distance_estimation.py`

Kamera tabanlı (**monocular**) mesafe tahmini için yardımcı fonksiyonları içerir.

**Şu anki yaklaşım:**
Bounding box boyutu ve bilinen gerçek hedef ölçülerine göre mesafe tahmini yapılmaktadır.

**İçerik:**

- Bilinen genişliğe göre mesafe tahmini
- Bilinen yüksekliğe göre mesafe tahmini
- Basit mesafe güven skoru hesaplama
- Tek hedef için mesafe tahmini
- Çoklu hedefler için toplu mesafe tahmini
- Kalibrasyon için focal length hesaplama yardımcı fonksiyonu

---

### `test.py`

Şu ana kadar geliştirilen modüllerin örnek kullanımını içerir.

**İçerik:**

- Örnek `DetectedObject` oluşturma
- Örnek focal length hesaplama
- Mesafe tahmini denemesi
- Önceliklendirme testi
- Matching algoritması test çağrısı

---

### `DataPrep`

Veri seti hazırlama aşaması için kullanılan yardımcı scriptleri içerir.

**İçerik:**

- Videodan her N'inci frame alma: `frame_extraction.py`
- Frame index takibi için: `last_index.txt`
- `last_index.txt` sıfırlama scripti

---

## Önemli Notlar

- Kodlar halen **erken aşamadadır** ve aktif geliştirme altındadır.
- Dosya isimleri ve modül yapısı ilerleyen aşamalarda yeniden düzenlenebilir.
- Mesafe tahmini için kullanılan hedef ölçüleri şu an **placeholder / yaklaşık değerlerdir**.
- Gerçek test ölçümleri ile güncellenmeleri gerekmektedir.
- Şimdilik lütfen dosyalar üzerinde değişiklik yapmayınız.

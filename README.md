## Мета роботи 
Дослідження особливостей реалізації цифрового підпису та сертифікатів. 

Було реалізовано алгоритм хешування SHA256 мовою С++. Звірено згідно тестових векторів за посиланням [DI Management](https://www.di-mgt.com.au/sha_testvectors.html)
Приклад використання: 

```с++ 
SHA256 sha256; 
const std::string str "abc"; 
sha256.update(str); 
std::arraysuint8_t, 32> hash sha256.digest(); //expected ba7816bf 8f01cfea 414140de 5dae2223 b00361a3 96177a9c b410ff61 f20015ad
```

В фукції main() наведено brute force перебор 20 байтового хешу. Перебор було зроблено на 1 потоку процесора Ryzen 7950х. Результуючий префікс: 614a1182f731d942863a3e9ab7f68319629741e4 було отримано через 34 хвилини за 631932159 спроб.

![image](https://github.com/user-attachments/assets/be95cd06-8e59-459d-9b42-bbe485cdce72)

## Генерація RSA ключової пари
```openssl genpkey -algorithm RSA -out server.key -pkeyopt rsa_keygen_bits:8192```
## Створення запиту на формування сертифіката (CSR)
```openssl req -new -key server.key -out server.csr -subj "/C=UA/L=Kyiv/O=KSE/CN=www.crypto.kse.ua emailAddress=user@kse.org.ua" nodes ```
## Перевірка вмісту CSR
```openssl req -text -noout -in server.csr```
## Створення самопідписаного сертифіката
```openssl x509 -req -sha256 -in server.csr -signkey server.key -out server.crt```
## Додавання сертифіката до довірених у системі
```certutil -addstore "Root" server.crt```

## Підписання запиту на сертифікат 
openssl x509 -req -sha256 -days 365 -in server_m.csr -CA server.crt -CAkey server.key -CAcreateserial -out server_m.crt
### Верифікація сертифіката:
![image](https://github.com/user-attachments/assets/cda2e8c4-6ddd-47c0-9f1e-4b697b5ce0a7)

------------------------

![image](https://github.com/user-attachments/assets/9599ac03-12a3-4c0f-88b5-dd6c7d857b59)

## Підпис та верифікація повідомлення
```openssl dgst -sha256 -binary text.txt > text.sha256```
## Підпис повідомлення за схемою 
```openssl pkeyutl -sign -inkey server.key -in text.sha256 -out sig_pkcs1.bin -pkeyopt digest:sha256 -pkeyopt rsa_padding_mode:pkcs1```
```openssl pkeyutl -sign -inkey server.key -in text.sha256 -out sig_pss.bin -pkeyopt digest:sha256 -pkeyopt rsa_padding_mode:pss -pkeyopt rsa_pss_saltlen:-1```
## RSA-шифрування повідомлення 
```openssl pkeyutl -encrypt -pubin -inkey key.pub -in text.txt -out message.enc```







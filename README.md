Мета роботи 

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

Переданные значения пустые
```C
printf("Err in %s:\nSome data is empty\n", __func__);
```

Ошибка выделения памяти
```C
printf("Err in %s:\nMemory was not allocated\n", __func__);
```

Данные не найдены
```C
printf("Err in %s:\nData was not found\n", __func__);
```

Некорректные данные
```C
printf("Err in %s:\nInvalid data\n", __func__);
```

Не открывается файл
```C
printf("Err in %s:\nFailed to open file %s\n", __func__, path);
```

Что-то другое
```C
printf("Err in %s:\nSomething went wrong\n", __func__);
```
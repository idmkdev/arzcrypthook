# ARZCryptHook

Исправление пакетов с клиента Arizona Mobile, для игры на sa:mp серверах

## Описание

Хук kyretardizeDatagram (на самом деле хукая SocketLayer::SendTo где он используется) для использования стандартной sa:mp 0.3.7 шифровки\
На данный момент поддерживает только armeabi-v7a (x32) версию игры

## Требования

- Android NDK

## Сборка

1. Клонируйте репозиторий:
```bash
git clone https://github.com/idmkdev/arzcrypthook.git
cd arzcrypthook
```

2. Убедитесь что у вас есть ndk-build
```bash
ndk-build --version
```

3. Соберите проект:
```bash
cd jni
ndk-build
```

После успешной сборки, библиотека будет находиться в директории `libs/(arch)`.

## Использование

1. Поместите библиотеку к остальным библиотекам приложения в /lib/(arch)
2. Добавьте загрузку библиотеки (после загрузки сампа): для Arizona Mobile это скорее всего com/arizona/game/GTASAInternal.smali
```smali
    const-string v1, "samp"

    invoke-static {v1}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V

    const-string v1, "packet_hook"

    invoke-static {v1}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V
```
## Авторы

- Radare - [@idmkdev](https://github.com/idmkdev)

##

Сделано для [ARZFUN](https://arzfun.com)
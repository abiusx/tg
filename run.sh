rm -rf grab/*
mkdir -p grab/_html/avatars
cp default.jpg grab/_html/avatars/
make && bin/telegram-cli

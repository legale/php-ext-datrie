# DATRIE PHP EXTENSION v0.0.1
## PHP 7+ implementation of the libdatrie C library 

## REQUIREMENTS
### packages:
  - php7.0-dev or later
  - git

## INSTALLATION
1. Clone this repository:<br>
run: `git clone https://github.com/legale/php-ext-datrie`
2. Move to the created directory:<br>
run: `cd php-ext-datrie`
3. Create `./configure` script from the configuration file `config.m4`:<br>
run: `phpize`
4. Make the extension:<br>
run: `make`
5. Install just maked datrie.so to the extension directory:<br>
run: `sudo make install`
6. Now you need to enable extension.
  - Add this string to the php configuration file `php.ini`:<br>
`extension=datrie.so`
  - Otherwise you may create `datrie.ini` file in 
the `/etc/php/{$PHP_VERSION}/mods-available` directory.<br>

  `datrie.ini`
  ```
  ; priority=100
  extension=datrie.so
  ``` 
  Now you can enable/disable extension with these commands:<br>
    - `sudo phpenmod datrie` to enable extension.
    - `sudo phpdismod datrie` to disable extension.

## DEMO
This demo shows basic library usage.<br>
run: ```php demo.php```

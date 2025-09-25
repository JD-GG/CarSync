-- Adminer 5.4.0 MariaDB 12.0.2-MariaDB-ubu2404 dump

SET NAMES utf8;
SET time_zone = '+00:00';
SET foreign_key_checks = 0;
SET sql_mode = 'NO_AUTO_VALUE_ON_ZERO';

SET NAMES utf8mb4;

CREATE DATABASE `db` /*!40100 DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_uca1400_ai_ci */;
USE `db`;

DROP TABLE IF EXISTS `benutzer`;
CREATE TABLE `benutzer` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(255) NOT NULL,
  `password` varchar(255) NOT NULL,
  `macInt` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `username` (`username`),
  UNIQUE KEY `macInt` (`macInt`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;

INSERT INTO `benutzer` (`id`, `username`, `password`, `macInt`) VALUES
(2,	'aaron',	'$2b$10$wKF6jnYlVKhtzaShPtReYuyKCObtRUnDLDatR5nlodri8Y/jddH4q',	187650260847649),
(4,	'hans.peter',	'$2b$10$jkBdZfZQpET9DXJJ325KIuRjbi257XxhR0r1BOZESaXC2XyHvDtYW',	187723572646452),
(5,	'luca.mueller',	'$2b$10$wNbT1aqvHDaGF4/HF6VU8Om7EN8duqUQCX8bGSDSzCwRnkyneq5h6',	74893038503804),
(6,	'JD2',	'$2b$10$aL5Zfhz9KTOnxPPpj7KCnOSAUUX8EQ3z6ulagU5hXoxCEr1Jn2ebq',	44350628295052),
(7,	'JD3',	'$2b$10$L21WLMB7qSd.NGe7VkFmcushVUzfLtTt0GeMKUoTVsw9NGn1Hz9D6',	141784827836400);

-- 2025-09-25 18:14:11 UTC


CREATE TABLE Electromenager (
    id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT,
    marque VARCHAR(40) NOT NULL,
    nom VARCHAR(50)NOT NULL,
    reference VARCHAR(30) NOT NULL,
    PRIMARY KEY (id)
)
CREATE TABLE Bricolage (
id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT,
   marque VARCHAR(40) NOT NULL,
    nom VARCHAR(50)NOT NULL,
    reference VARCHAR(30) NOT NULL,
    PRIMARY KEY (id)
)
CREATE TABLE Jardin (
id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT,
   marque VARCHAR(40) NOT NULL,
    nom VARCHAR(50)NOT NULL,
    reference VARCHAR(30) NOT NULL,
    PRIMARY KEY (id)
)
CREATE TABLE Menage (
id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT,
   marque VARCHAR(40) NOT NULL,
    nom VARCHAR(50)NOT NULL,
    reference VARCHAR(30) NOT NULL,
    PRIMARY KEY (id)
)
INSERT INTO Electromenager (marque, nom, reference)
VALUES 
(1, 'Moulinex', 'robot pâtissier gourmet', 'QA200110'),
(2, 'Moulinex', 'hachoir fresh express 3 cones', 'DJ753315' ),
(3, 'Krups', 'espresso dolce piccolo noir', 'YY1049FD'),
(4, 'Tefal', 'plancha thermospot 2000W', 'CB540812'),
(5, 'Seb', 'cocotte clipso one 8L fleur', 'P4311401');

INSERT INTO Bricolage (marque, nom, reference)
VALUES
(1, 'Vachette', 'verrou eclador a boutton', '67632/5C'),
(2, 'Loctite', 'colle super glue liquide', '793855'),
(3, 'Geb', 'gebsicone W BLCIFONG', '893150'),
(4, 'Stanley', 'scie egoine jet cut', '2-15-599'),
(5, 'Makita', 'meuleuse 115mm 720W', 'GA4530');

INSERT INTO Jardin (marque, nom, reference)
VALUES
(1, 'Vilmorin', 'terreau univers 70L', '6401091'),
(2, 'Gardena', 'pulverisateur pression 5L', '823-27'),
(3, 'Gardena', 'laque scolo fourmis 1L', 'LAQ57'),
(4, 'Fertilgène', 'engrais fleur jardin 750G', 'OSFLEUR'),
(5, 'Fertiligène', 'engrais rosier arbust 750G', 'OSROS');

INSERT INTO Menage (marque, nom, reference)
VALUES
(1, 'Teraillon', 'pese pers TX6000', ' ');


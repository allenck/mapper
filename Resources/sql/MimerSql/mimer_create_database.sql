-- 1. Create the storage databank (automatically saves to the database's home directory)
CREATE DATABANK mapper_db FILENAME 'mapper_db.dbf';

-- 2. Create the target user identity with standard user behaviors
CREATE IDENT mapperuser AS USER USING 'iic723_knobacres';

-- 3. Establish the formal schema namespace owned by mapperuser inside the new databank
CREATE SCHEMA mapperuser AUTHORIZATION mapperuser DEFAULT DATABANK mapper_db;

-- 4. Give mapperuser structural control to dynamically CREATE and DROP tables inside its space
GRANT DATABANK, SCHEMA TO mapperuser;

-- 5. Establish your separate administrative identity for password and user control
CREATE IDENT mapperadm AS USER USING 'iic723_knobacres';
GRANT IDENT TO mapperadm WITH GRANT OPTION;
GRANT DATABANK, SCHEMA TO mapperadm WITH GRANT OPTION;

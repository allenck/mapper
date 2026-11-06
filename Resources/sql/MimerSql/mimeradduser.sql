-- 1. Create the new administrative user
CREATE IDENT mapperadm AS USER USING 'iic723_knobacres';

-- 2. Give mapperadm global administrative privileges (including identity control)
GRANT IDENT TO mapperadm WITH GRANT OPTION;
GRANT DATABANK, SCHEMA TO mapperadm WITH GRANT OPTION;

-- 3. Strip password-changing rights from mapperuser by forcing a standard USER profile
ALTER IDENT mapperuser AS USER USING 'iic723_knobacres';

-- 4. Grant mapperuser the specific rights required to create and drop tables for exports
GRANT DATABANK, SCHEMA TO mapperuser;

BEGIN;

CREATE TYPE "Position" AS ENUM
(
    'Unknown',
    'Developer',
    'Manager'
);

CREATE TABLE employees
(
    employee_id bigserial not null primary key,
    name varchar not null,
    last_name varchar not null,
    age integer not null,
    email varchar not null,
    position "Position" not null default 'Unknown',
    rate numeric(10, 2) not null
);

COMMIT;

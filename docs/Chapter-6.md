# Database support in MIF

To work with databases, MIF provides three interfaces:
- IConnection
- IStatement
- IRecordset  

All interfaces have their own implementation for each type of database. All implementations are MIF-services. To create the implementation you need, you must use the Create function from the MIF framework, passing the implementation ID and other necessary parameters to it. In addition, MIF provides a connection pool implementation in which you do not have to worry about creating a connection and reconnecting when the connection is broken. Take a look at the following code
```cpp
auto connections = Mif::Service::Create
    <
        Mif::Db::Id::Service::PostgresConnectionPool,
        Mif::Db::IConnectionPool
    >(config->GetConfig("database"));
```
This code demonstrates the creation of a connection pool to PostgreSQL from the configuration branch. Below you can see the configuration branch for this
```xml
<database>
    <host>127.0.0.1</host>
    <port>5432</port>
    <dbname>test_phonebook</dbname>
    <user>postgres</user>
    <password></password>
    <connectiontimeout>30</connectiontimeout>
</database>
```
In addition, MIF also has other methods for creating a connection pool that can be used without a configuration branch simply by passing parameters to the database connection.  

Below you can see a code snippet that demonstrates all database support in MIF
```cpp
virtual void Set(std::string const &phone, std::string const &name) override final
{
    auto conn = m_connections->GetConnection();
    Mif::Db::Transaction tr{conn};

    auto stmt = conn->CreateStatement(
            "insert into phones (phone, name) "
            "values ($1::text, $2::text) "
            "returning phone; "
        );

    auto res = stmt->Execute({Quote(phone), Quote(name)});

    if (!res->Read())
        throw std::runtime_error{"Failed to put phone number \"" + phone + "\" for name \"" + name + "\""};

    tr.Commit();
}
```
The above code is part of the sample phone book service. This method inserts a new record into the database and verifies the success of the insert. In the code above, you can also see the use of transactions.  

You will find all the code in example [phonebook_service](https://github.com/tdv/mif/tree/master/examples/_doc/chapters/chapter6/phonebook_service).  

This example contains two implementations of the IStorage interface for storing data in a file or PostgreSQL. The implementation is selected in the configuration file.  

Currently, MIF has an implementation for SQLite and PostgreSQL databases. In addition to the above example, you can see more about SQLite and PostgreSQL support in example [db_client](https://github.com/tdv/mif/tree/master/examples/db_client).  

_**Congratulations! Now you can create full-featured web services with database support. This is all you need to create your own MIF-based web services.**_
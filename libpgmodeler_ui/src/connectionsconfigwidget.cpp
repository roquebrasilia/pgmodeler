/*
# PostgreSQL Database Modeler (pgModeler)
#
# Copyright 2006-2014 - Raphael Araújo e Silva <rkhaotix@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# The complete text of GPLv3 is at LICENSE file on source code root directory.
# Also, you can get the complete GNU General Public License at <http://www.gnu.org/licenses/>
*/

#include "connectionsconfigwidget.h"

ConnectionsConfigWidget::ConnectionsConfigWidget(QWidget * parent) : QWidget(parent)
{
	setupUi(this);

	connect(ssl_mode_cmb, SIGNAL(currentIndexChanged(int)), this, SLOT(enableCertificates(void)));

	connect(new_tb, SIGNAL(clicked(bool)), this, SLOT(newConnection(void)));
	connect(cancel_tb, SIGNAL(clicked(bool)), this, SLOT(newConnection(void)));

	connect(duplicate_tb, SIGNAL(clicked(bool)), this, SLOT(duplicateConnection(void)));

	connect(test_tb, SIGNAL(clicked(bool)), this, SLOT(testConnection(void)));
	connect(add_tb, SIGNAL(clicked(bool)), this, SLOT(handleConnection(void)));
	connect(update_tb, SIGNAL(clicked(bool)), this, SLOT(handleConnection(void)));
	connect(edit_tb, SIGNAL(clicked(bool)), this, SLOT(editConnection(void)));
	connect(remove_tb, SIGNAL(clicked(bool)), this, SLOT(removeConnection(void)));

	connect(alias_edt, SIGNAL(textChanged(QString)), this, SLOT(enableConnectionTest(void)));
	connect(host_edt, SIGNAL(textChanged(QString)), this, SLOT(enableConnectionTest(void)));
	connect(user_edt, SIGNAL(textChanged(QString)), this, SLOT(enableConnectionTest(void)));
	connect(passwd_edt, SIGNAL(textChanged(QString)), this, SLOT(enableConnectionTest(void)));
	connect(conn_db_edt, SIGNAL(textChanged(QString)), this, SLOT(enableConnectionTest(void)));

	update_tb->setVisible(false);
	cancel_tb->setVisible(false);
}

ConnectionsConfigWidget::~ConnectionsConfigWidget(void)
{
	//Destroy all the loaded connections
	while(connections_cmb->count() > 0)
		this->removeConnection();
}

void ConnectionsConfigWidget::hideEvent(QHideEvent *)
{
	this->newConnection();
}

void ConnectionsConfigWidget::loadConfiguration(void)
{
	vector<QString> key_attribs;
	map<QString, attribs_map >::iterator itr, itr_end;
	Connection *conn=nullptr;

	//Destroy all the loaded connections
	while(connections_cmb->count() > 0)
		this->removeConnection();

	key_attribs.push_back(ParsersAttributes::ALIAS);
	BaseConfigWidget::loadConfiguration(GlobalAttributes::CONNECTIONS_CONF, key_attribs);

	itr=config_params.begin();
	itr_end=config_params.end();

	while(itr!=itr_end)
	{
		conn=new Connection;

		conn->setConnectionParam(Connection::PARAM_SERVER_FQDN, itr->second[Connection::PARAM_SERVER_FQDN]);
		conn->setConnectionParam(Connection::PARAM_PORT, itr->second[Connection::PARAM_PORT]);
		conn->setConnectionParam(Connection::PARAM_USER, itr->second[Connection::PARAM_USER]);
		conn->setConnectionParam(Connection::PARAM_PASSWORD,itr->second[Connection::PARAM_PASSWORD]);
		conn->setConnectionParam(Connection::PARAM_DB_NAME, itr->second[Connection::PARAM_DB_NAME]);
		conn->setConnectionParam(Connection::PARAM_CONN_TIMEOUT, itr->second[Connection::PARAM_CONN_TIMEOUT]);
		conn->setConnectionParam(Connection::PARAM_SSL_MODE, itr->second[Connection::PARAM_SSL_MODE]);
		conn->setConnectionParam(Connection::PARAM_SSL_ROOT_CERT, itr->second[Connection::PARAM_SSL_ROOT_CERT]);
		conn->setConnectionParam(Connection::PARAM_SSL_CERT, itr->second[Connection::PARAM_SSL_CERT]);
		conn->setConnectionParam(Connection::PARAM_SSL_KEY, itr->second[Connection::PARAM_SSL_KEY]);
		conn->setConnectionParam(Connection::PARAM_SSL_CRL, itr->second[Connection::PARAM_SSL_CRL]);
		conn->setConnectionParam(Connection::PARAM_LIB_GSSAPI, itr->second[Connection::PARAM_LIB_GSSAPI]);
		conn->setConnectionParam(Connection::PARAM_KERBEROS_SERVER, itr->second[Connection::PARAM_KERBEROS_SERVER]);
		conn->setConnectionParam(Connection::PARAM_OPTIONS, itr->second[Connection::PARAM_OPTIONS]);

		connections_cmb->addItem(Utf8String::create(itr->second[ParsersAttributes::ALIAS]) +
				QString(" (%1:%2)").arg(itr->second[Connection::PARAM_SERVER_FQDN]).arg(itr->second[Connection::PARAM_PORT]),
				QVariant::fromValue<void *>(reinterpret_cast<void *>(conn)));

		itr++;
	}

	edit_tb->setEnabled(connections_cmb->count() > 0);
	remove_tb->setEnabled(connections_cmb->count() > 0);
}

void ConnectionsConfigWidget::enableCertificates(void)
{
	client_cert_lbl->setEnabled(ssl_mode_cmb->currentIndex()!=0);
	client_cert_edt->setEnabled(ssl_mode_cmb->currentIndex()!=0);
	root_cert_lbl->setEnabled(ssl_mode_cmb->currentIndex()!=0);
	root_cert_edt->setEnabled(ssl_mode_cmb->currentIndex()!=0);
	crl_edt->setEnabled(ssl_mode_cmb->currentIndex()!=0);
	crl_lbl->setEnabled(ssl_mode_cmb->currentIndex()!=0);
	client_key_lbl->setEnabled(ssl_mode_cmb->currentIndex()!=0);
	client_key_edt->setEnabled(ssl_mode_cmb->currentIndex()!=0);
}

void ConnectionsConfigWidget::enableConnectionTest(void)
{
	test_tb->setEnabled(!alias_edt->text().isEmpty() &&
											!host_edt->text().isEmpty() &&
											!user_edt->text().isEmpty() &&
											!passwd_edt->text().isEmpty() &&
											!conn_db_edt->text().isEmpty());
	add_tb->setEnabled(test_tb->isEnabled());
  update_tb->setEnabled(test_tb->isEnabled());
}

void ConnectionsConfigWidget::newConnection(void)
{
	conn_db_edt->clear();
	alias_edt->clear();
	user_edt->clear();
	host_edt->clear();
	port_sbp->setValue(5432);
	passwd_edt->clear();
	options_edt->clear();

	ssl_mode_cmb->setCurrentIndex(0);
	client_cert_edt->setText("~/.postgresql/postgresql.crt");
	root_cert_edt->setText("~/.postgresql/root.crt");
	crl_edt->setText("~/.postgresql/root.crl");
	client_key_edt->setText("~/.postgresql/postgresql.key");

	gssapi_auth_chk->setChecked(false);
	krb_server_edt->clear();

	timeout_sbp->setValue(2);
	add_tb->setVisible(true);
	update_tb->setVisible(false);
	connections_cmb->setEnabled(true);

	new_tb->setVisible(true);
	cancel_tb->setVisible(false);

	edit_tb->setEnabled(connections_cmb->count() > 0);
	remove_tb->setEnabled(connections_cmb->count() > 0);
	duplicate_tb->setEnabled(connections_cmb->count() > 0);
}

void ConnectionsConfigWidget::duplicateConnection()
{
	Connection *conn=nullptr, *new_conn=nullptr;
	QString alias;

	try
	{
		conn=reinterpret_cast<Connection *>(connections_cmb->itemData(connections_cmb->currentIndex()).value<void *>());
		new_conn=new Connection;
		(*new_conn)=(*conn);
		alias=QString("cp_%1").arg(connections_cmb->currentText());
		connections_cmb->addItem(alias,  QVariant::fromValue<void *>(reinterpret_cast<void *>(new_conn)));
		connections_cmb->setCurrentIndex(connections_cmb->count()-1);
	}
	catch(Exception &e)
	{
		if(new_conn)
			delete(new_conn);

		throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void ConnectionsConfigWidget::handleConnection(void)
{
	Connection *conn=nullptr;
	QString alias;

	try
	{
		alias=QString("%1 (%2:%3)").arg(alias_edt->text()).arg(host_edt->text()).arg(port_sbp->value());

		if(!update_tb->isVisible())
		{
			conn=new Connection;
			this->configureConnection(conn);
			connections_cmb->addItem(alias, QVariant::fromValue<void *>(reinterpret_cast<void *>(conn)));
		}
		else
		{
			conn=reinterpret_cast<Connection *>(connections_cmb->itemData(connections_cmb->currentIndex()).value<void *>());
			this->configureConnection(conn);
			connections_cmb->setItemText(connections_cmb->currentIndex(), alias);
		}

		this->newConnection();
		edit_tb->setEnabled(connections_cmb->count() > 0);
		remove_tb->setEnabled(connections_cmb->count() > 0);
	}
	catch(Exception &e)
	{
		if(add_tb->isVisible())
			delete(conn);

		throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void ConnectionsConfigWidget::removeConnection(void)
{
	if(connections_cmb->currentIndex() >= 0)
	{
		Connection *conexao=nullptr;

		conexao=reinterpret_cast<Connection *>(connections_cmb->itemData(connections_cmb->currentIndex()).value<void *>());
		connections_cmb->removeItem(connections_cmb->currentIndex());
		delete(conexao);
		this->newConnection();
	}
}

void ConnectionsConfigWidget::editConnection(void)
{
	if(connections_cmb->count() > 0)
	{
		Connection *conn=nullptr;

		conn=reinterpret_cast<Connection *>(connections_cmb->itemData(connections_cmb->currentIndex()).value<void *>());

		//Removes the (host:port) portion from the alias before fill the field
		alias_edt->setText(connections_cmb->currentText().remove(QRegExp("( )(\\()(.)+(\\:)(.)+(\\))")));

		if(!conn->getConnectionParam(Connection::PARAM_SERVER_FQDN).isEmpty())
			host_edt->setText(conn->getConnectionParam(Connection::PARAM_SERVER_FQDN));
		else
			host_edt->setText(conn->getConnectionParam(Connection::PARAM_SERVER_IP));

		conn_db_edt->setText(conn->getConnectionParam(Connection::PARAM_DB_NAME));
		user_edt->setText(conn->getConnectionParam(Connection::PARAM_USER));
		passwd_edt->setText(conn->getConnectionParam(Connection::PARAM_PASSWORD));
		port_sbp->setValue(conn->getConnectionParam(Connection::PARAM_PORT).toInt());
		timeout_sbp->setValue(conn->getConnectionParam(Connection::PARAM_CONN_TIMEOUT).toInt());

		krb_server_edt->setText(conn->getConnectionParam(Connection::PARAM_KERBEROS_SERVER));
		gssapi_auth_chk->setChecked(conn->getConnectionParam(Connection::PARAM_LIB_GSSAPI)=="gssapi");
		options_edt->setText(conn->getConnectionParam(Connection::PARAM_OPTIONS));

		if(conn->getConnectionParam(Connection::PARAM_SSL_MODE)==Connection::SSL_DESABLE)
			ssl_mode_cmb->setCurrentIndex(0);
		else if(conn->getConnectionParam(Connection::PARAM_SSL_MODE)==Connection::SSL_ALLOW)
			ssl_mode_cmb->setCurrentIndex(1);
		else if(conn->getConnectionParam(Connection::PARAM_SSL_MODE)==Connection::SSL_REQUIRE)
			ssl_mode_cmb->setCurrentIndex(2);
		else if(conn->getConnectionParam(Connection::PARAM_SSL_MODE)==Connection::SSL_CA_VERIF)
			ssl_mode_cmb->setCurrentIndex(3);
		else
			ssl_mode_cmb->setCurrentIndex(4);

		if(ssl_mode_cmb->currentIndex() > 0)
		{
			client_cert_edt->setText(conn->getConnectionParam(Connection::PARAM_SSL_CERT));
			root_cert_edt->setText(conn->getConnectionParam(Connection::PARAM_SSL_ROOT_CERT));
			client_key_edt->setText(conn->getConnectionParam(Connection::PARAM_SSL_KEY));
			crl_edt->setText(conn->getConnectionParam(Connection::PARAM_SSL_CRL));
		}

		update_tb->setVisible(true);
		add_tb->setVisible(false);
		connections_cmb->setEnabled(false);

		new_tb->setVisible(false);
		duplicate_tb->setEnabled(false);
		cancel_tb->setVisible(true);
		edit_tb->setEnabled(false);
	}
}

void ConnectionsConfigWidget::configureConnection(Connection *conn)
{
	if(conn)
	{
		conn->setConnectionParam(Connection::PARAM_SERVER_FQDN, host_edt->text());
		conn->setConnectionParam(Connection::PARAM_PORT, QString("%1").arg(port_sbp->value()));
		conn->setConnectionParam(Connection::PARAM_USER, user_edt->text());
		conn->setConnectionParam(Connection::PARAM_PASSWORD, passwd_edt->text());
		conn->setConnectionParam(Connection::PARAM_DB_NAME, conn_db_edt->text());
		conn->setConnectionParam(Connection::PARAM_CONN_TIMEOUT, QString("%1").arg(timeout_sbp->value()));

		switch(ssl_mode_cmb->currentIndex())
		{
			case 1:
				conn->setConnectionParam(Connection::PARAM_SSL_MODE, Connection::SSL_ALLOW);
			break;
			case 2:
				conn->setConnectionParam(Connection::PARAM_SSL_MODE, Connection::SSL_REQUIRE);
			break;
			case 3:
				conn->setConnectionParam(Connection::PARAM_SSL_MODE, Connection::SSL_CA_VERIF);
			break;
			case 4:
				conn->setConnectionParam(Connection::PARAM_SSL_MODE, Connection::SSL_FULL_VERIF);
			break;
			default:
			case 0:
				conn->setConnectionParam(Connection::PARAM_SSL_MODE, Connection::SSL_DESABLE);
			break;
		}

		if(ssl_mode_cmb->currentIndex()!=0)
		{
			conn->setConnectionParam(Connection::PARAM_SSL_ROOT_CERT, root_cert_edt->text());
			conn->setConnectionParam(Connection::PARAM_SSL_CERT, client_cert_edt->text());
			conn->setConnectionParam(Connection::PARAM_SSL_KEY, client_key_edt->text());
			conn->setConnectionParam(Connection::PARAM_SSL_CRL, crl_edt->text());
		}

		if(gssapi_auth_chk->isChecked())
			conn->setConnectionParam(Connection::PARAM_LIB_GSSAPI, "gssapi");

		if(!krb_server_edt->text().isEmpty())
			conn->setConnectionParam(Connection::PARAM_KERBEROS_SERVER, krb_server_edt->text());

		if(!options_edt->text().isEmpty())
			conn->setConnectionParam(Connection::PARAM_OPTIONS, options_edt->text());
	}
}

void ConnectionsConfigWidget::testConnection(void)
{
	Connection conn;
	Messagebox msg_box;

	try
	{
		this->configureConnection(&conn);
		conn.connect();
		msg_box.show(trUtf8("Success"), trUtf8("Connection successfuly stablished!"), Messagebox::INFO_ICON);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void ConnectionsConfigWidget::restoreDefaults(void)
{
	try
	{
		//Restore the default connection config file
		BaseConfigWidget::restoreDefaults(GlobalAttributes::CONNECTIONS_CONF);

		//Remove all connections
		while(connections_cmb->count() > 0)
			this->removeConnection();

		//Reloads the configuration
		this->loadConfiguration();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void ConnectionsConfigWidget::saveConfiguration(void)
{
	try
	{
		int i, count;
		Connection *conn=nullptr;
		attribs_map attribs;

    /* If add or update buttons are enabled when saving the configs indicates
       that user forgot to click on these buttons and register the connection,
       so in order to do not lost the data pgModeler will ask to save the connection. */
    if(add_tb->isEnabled() || update_tb->isEnabled())
    {
      Messagebox msg_box;

      msg_box.show(trUtf8("Alert"), trUtf8("There is an unsaved connection! Want to save it?"),
               Messagebox::ALERT_ICON, Messagebox::YES_NO_BUTTONS);

      if(msg_box.result()==QDialog::Accepted)
       handleConnection();
    }

		config_params[GlobalAttributes::CONNECTIONS_CONF].clear();
		count=connections_cmb->count();

		/* Workaround: When there is no connection, to prevent saving an empty file, is necessary to
		 fill the attribute CONNECTIONS with white spaces */
		if(count==0)
			config_params[GlobalAttributes::CONNECTIONS_CONF][ParsersAttributes::CONNECTIONS]="  ";
		else
		{
			for(i=0; i < count; i++)
			{
				conn=reinterpret_cast<Connection *>(connections_cmb->itemData(i).value<void *>());
				attribs=conn->getConnectionParams();

				if(attribs[Connection::PARAM_SERVER_FQDN].isEmpty())
					attribs[Connection::PARAM_SERVER_FQDN]=attribs[Connection::PARAM_SERVER_IP];

				attribs[ParsersAttributes::ALIAS]=connections_cmb->itemText(i).remove(QRegExp(" \\((.)*\\)"));

				SchemaParser::setIgnoreUnkownAttributes(true);
				config_params[GlobalAttributes::CONNECTIONS_CONF][ParsersAttributes::CONNECTIONS]+=
						SchemaParser::getCodeDefinition(GlobalAttributes::CONFIGURATIONS_DIR +
																						GlobalAttributes::DIR_SEPARATOR +
																						GlobalAttributes::SCHEMAS_DIR +
																						GlobalAttributes::DIR_SEPARATOR +
																						GlobalAttributes::CONNECTIONS_CONF +
																						GlobalAttributes::SCHEMA_EXT,
																						attribs);
				SchemaParser::setIgnoreUnkownAttributes(false);
			}
		}

		BaseConfigWidget::saveConfiguration(GlobalAttributes::CONNECTIONS_CONF);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void ConnectionsConfigWidget::getConnections(map<QString, Connection *> &conns, bool inc_hosts)
{
	int i, count;
	QString alias;

	conns.clear();
	count=connections_cmb->count();

	for(i=0; i < count; i++)
	{
		alias=connections_cmb->itemText(i);

		if(!inc_hosts)
			alias.remove(QRegExp(" \\((.)*\\)"));

		conns[alias]=reinterpret_cast<Connection *>(connections_cmb->itemData(i).value<void *>());
	}
}



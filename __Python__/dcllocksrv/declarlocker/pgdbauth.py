# -*- coding: utf-8 -*-
import logging

from sqlalchemy.engine import create_engine
from sqlalchemy.exc import StatementError

try:
    from sqlalchemy.ext.declarative.api import declarative_base
except ImportError:
    from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm.session import sessionmaker
from twisted.cred import credentials, error
from twisted.cred.checkers import ICredentialsChecker
from twisted.internet import defer
from twisted.python import failure
from zope.interface.declarations import implements


__author__ = 'Savenko'

Base = declarative_base()


class PgPasswordDB:
    implements(ICredentialsChecker)

    credentialInterfaces = (credentials.IUsernamePassword,
                            credentials.IUsernameHashedPassword)

    def __init__(self, dbstr):
        engine = create_engine(dbstr, echo=(logging.root.level == logging.DEBUG))
        metadata = Base.metadata
        metadata.reflect(bind=engine, only=('users',))
        self.users = metadata.tables['users']
        self.session = sessionmaker(bind=engine)()

    def _cbPasswordMatch(self, matched, username):
        if matched:
            return username
        else:
            return failure.Failure(error.UnauthorizedLogin())

    def requestAvatarId(self, credentials):
        try:
            user = self.session.query(self.users).filter_by(
                dblogin=credentials.username
            ).first()
        except StatementError:
            # try to reconnect
            self.session.rollback()
            try:
                user = self.session.query(self.users).filter_by(
                    dblogin=credentials.username
                ).first()
            except Exception as e:
                logging.error(e.message)
        finally:
            self.session.rollback()

        if not user is None:
            try:
                return defer.maybeDeferred(
                    credentials.checkPassword,
                    user.rightpass).addCallback(
                    self._cbPasswordMatch, str(credentials.username))
            except Exception as e:
                logging.error(e.message)
                return defer.fail(error.UnauthorizedLogin())
        else:
            return defer.fail(error.UnauthorizedLogin())
